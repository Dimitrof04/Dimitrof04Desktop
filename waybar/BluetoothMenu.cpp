#include <gtkmm.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <algorithm>
#include <memory>

struct BluetoothDevice {
    std::string mac;
    std::string name;
    bool connected;
    bool paired;
};

std::string exec_cmd(const char *cmd)
{
    char buffer[128];
    std::string result = "";
    FILE *pipe = popen(cmd, "r");
    if (!pipe)
        return "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

bool is_bluetooth_on()
{
    std::string out = exec_cmd("bluetoothctl show");
    return out.find("Powered: yes") != std::string::npos;
}

std::vector<BluetoothDevice> get_bluetooth_devices()
{
    std::vector<BluetoothDevice> devices;
    std::set<std::string> seen_macs; // Evita duplicados entre as duas listas

    // Lista 1: Todos os dispositivos conhecidos/detectados recentemente
    std::string out_all = exec_cmd("bluetoothctl devices");
    // Lista 2: Dispositivos especificamente pareados (garantia extra)
    std::string out_paired = exec_cmd("bluetoothctl devices Paired");
    
    std::stringstream ss(out_all + "\n" + out_paired);
    std::string line;

    while (std::getline(ss, line))
    {
        if (line.find("Device ") == 0)
        {
            std::string mac = line.substr(7, 17);
            std::string name = line.substr(25);
            
            if (seen_macs.find(mac) != seen_macs.end())
                continue;
                
            seen_macs.insert(mac);

            std::string info = exec_cmd(("bluetoothctl info " + mac).c_str());
            bool connected = (info.find("Connected: yes") != std::string::npos);
            bool paired = (info.find("Paired: yes") != std::string::npos);

            devices.push_back({mac, name, connected, paired});
            if (devices.size() >= 6)
                break;
        }
    }
    return devices;
}

Gtk::Box* p_devices_box = nullptr;
Gtk::Window* p_window = nullptr;

void refresh_device_list()
{
    if (!p_devices_box || !p_window || !is_bluetooth_on()) return;

    auto children = p_devices_box->get_children();
    for (auto child : children) {
        p_devices_box->remove(*child);
    }

    auto devices = get_bluetooth_devices();

    if (!devices.empty())
    {
        for (const auto &dev : devices)
        {
            std::string prefix = dev.connected ? "󰂱  " : (dev.paired ? "󰂲  " : "󰂰  ");
            auto dev_btn = Gtk::make_managed<Gtk::Button>(prefix + dev.name);
            dev_btn->get_style_context()->add_class("bt-list-btn");
            if (dev.connected) {
                dev_btn->get_style_context()->add_class("connected-device");
            }
            dev_btn->set_alignment(0.0, 0.5);
            p_devices_box->pack_start(*dev_btn);

            dev_btn->add_events(Gdk::BUTTON_PRESS_MASK);
            dev_btn->signal_button_press_event().connect([dev](GdkEventButton* event) -> bool {
                if (event->type == GDK_BUTTON_PRESS) 
                {
                    if (dev.connected) {
                        std::string cmd = "bluetoothctl disconnect " + dev.mac + " &";
                        std::system(cmd.c_str());
                    } else {
                        // Se não estiver pareado, parea primeiro. Se já estiver, só conecta.
                        std::string cmd;
                        if (!dev.paired) {
                            cmd = "bluetoothctl pair " + dev.mac + " && bluetoothctl trust " + dev.mac + " && bluetoothctl connect " + dev.mac + " &";
                        } else {
                            cmd = "bluetoothctl connect " + dev.mac + " &";
                        }
                        std::system(cmd.c_str());
                    }
                    Gtk::Main::quit();
                    return true;
                }
                else if (event->type == GDK_2BUTTON_PRESS) 
                {
                    std::string cmd = "bluetoothctl remove " + dev.mac + " &";
                    std::system(cmd.c_str());
                    Gtk::Main::quit();
                    return true;
                }
                return false;
            });
        }
    }
    else
    {
        auto lbl_none = Gtk::make_managed<Gtk::Label>("Buscando dispositivos...");
        lbl_none->set_halign(Gtk::ALIGN_START);
        p_devices_box->pack_start(*lbl_none);
    }

    p_window->show_all();
    p_window->resize(1, 1);
}

bool on_timeout()
{
    refresh_device_list();
    return true;
}

int main(int argc, char *argv[])
{
    auto app = Gtk::Application::create(argc, argv, "org.waybar.btmenu");
    Gtk::Window window;
    p_window = &window;

    GtkWindow *gtk_win = window.gobj();
    gtk_layer_init_for_window(gtk_win);
    gtk_layer_set_layer(gtk_win, GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_keyboard_interactivity(gtk_win, true);

    gtk_layer_set_anchor(gtk_win, GTK_LAYER_SHELL_EDGE_TOP, true);
    gtk_layer_set_anchor(gtk_win, GTK_LAYER_SHELL_EDGE_RIGHT, true);

    gtk_layer_set_margin(gtk_win, GTK_LAYER_SHELL_EDGE_TOP, 25);
    gtk_layer_set_margin(gtk_win, GTK_LAYER_SHELL_EDGE_RIGHT, 20);

    window.add_events(Gdk::LEAVE_NOTIFY_MASK | Gdk::BUTTON_PRESS_MASK);

    auto css = Gtk::CssProvider::create();
    css->load_from_data(
        "window { background-color: rgba(30, 30, 46, 0.95); border: 2px solid #ffffff; border-radius: 12px; }"
        "box { padding: 12px; }"
        "label.title { color: #a6adc8; font-weight: bold; font-size: 11px; }"
        "button { background: #313244; color: #cdd6f4; border: 2px solid transparent; border-radius: 6px; padding: 8px 16px; font-weight: bold; }"
        "button:hover { background: #ffffff; color: #11111b; }"
        ".bt-list-btn { background: #1e1e2e; font-weight: normal; border: 2px solid transparent; }"
        ".bt-list-btn:hover { background: #45475a; color: #ffffff; }"
        ".refresh-btn { background: transparent; color: #a6adc8; border: none; padding: 4px 8px; font-size: 12px; }"
        ".refresh-btn:hover { background: #313244; color: #ffffff; }"
        ".connected-device { color: #a6e3a1; }");
    
    auto screen = Gdk::Screen::get_default();
    Gtk::StyleContext::add_provider_for_screen(screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    Gtk::Box main_box(Gtk::ORIENTATION_VERTICAL, 8);

    bool bt_active = is_bluetooth_on();

    auto btn_toggle = Gtk::make_managed<Gtk::Button>(bt_active ? "󰂭  Desligar Bluetooth" : "󰂯  Ligar Bluetooth");
    btn_toggle->set_halign(Gtk::ALIGN_CENTER);
    btn_toggle->signal_clicked().connect([bt_active]() {
        if (bt_active) {
            std::system("bluetoothctl scan off &");
            std::system("bluetoothctl power off &");
        } else {
            std::system("bluetoothctl power on &");
        }
        Gtk::Main::quit();
    });
    main_box.pack_start(*btn_toggle);

    if (bt_active)
    {
        // ATIVA A BUSCA AUTOMÁTICA EM SEGUNDO PLANO
        std::system("bluetoothctl scan on &");

        auto header_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 0);
        
        auto lbl_list = Gtk::make_managed<Gtk::Label>("DISPOSITIVOS:");
        lbl_list->get_style_context()->add_class("title");
        lbl_list->set_halign(Gtk::ALIGN_START);
        
        auto btn_refresh = Gtk::make_managed<Gtk::Button>("󰑐");
        btn_refresh->get_style_context()->add_class("refresh-btn");
        btn_refresh->set_halign(Gtk::ALIGN_END);
        btn_refresh->signal_clicked().connect([]() { refresh_device_list(); });

        header_box->pack_start(*lbl_list, Gtk::PACK_EXPAND_WIDGET);
        header_box->pack_end(*btn_refresh, Gtk::PACK_SHRINK);
        main_box.pack_start(*header_box);

        auto devices_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 6);
        p_devices_box = devices_box;
        main_box.pack_start(*devices_box);

        refresh_device_list();

        Glib::signal_timeout().connect(sigc::ptr_fun(&on_timeout), 30000);
    }

    // Desliga o scan ao fechar a janela para não gastar bateria à toa
    window.signal_leave_notify_event().connect([&window](GdkEventCrossing *event) {
        if (event->mode == GDK_CROSSING_NORMAL && event->detail != GDK_NOTIFY_INFERIOR) {
            std::system("bluetoothctl scan off &");
            Gtk::Main::quit();
        }
        return false;
    });

    window.signal_focus_out_event().connect([&window](GdkEventFocus *) {
        std::system("bluetoothctl scan off &");
        Gtk::Main::quit();
        return false;
    });

    window.signal_key_press_event().connect([&window](GdkEventKey *key_event) {
        if (key_event->keyval == GDK_KEY_Escape) {
            std::system("bluetoothctl scan off &");
            Gtk::Main::quit();
            return true;
        }
        return false;
    });

    window.add(main_box);
    window.show_all();

    return app->run(window);
}