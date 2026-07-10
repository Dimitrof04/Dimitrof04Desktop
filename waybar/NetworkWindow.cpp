#include <gtkmm.h>
#include <gtk-layer-shell/gtk-layer-shell.h> // Biblioteca nativa do Wayland
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <algorithm>
#include <memory>

std::string exec_cmd(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

bool is_wifi_on() {
    std::string out = exec_cmd("/usr/bin/nmcli radio wifi");
    return out.find("enabled") != std::string::npos;
}

std::vector<std::string> get_wifi_networks() {
    std::system("/usr/bin/nmcli device wifi rescan > /dev/null 2>&1");
    std::string out = exec_cmd("/usr/bin/nmcli -f SSID dev wifi");
    
    std::stringstream ss(out);
    std::string line;
    std::vector<std::string> networks;
    std::set<std::string> seen;
    
    bool first = true;
    while (std::getline(ss, line)) {
        if (first) { first = false; continue; }
        
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (!line.empty() && line != "--" && seen.find(line) == seen.end()) {
            seen.insert(line);
            networks.push_back(line);
            if (networks.size() >= 6) break;
        }
    }
    std::sort(networks.begin(), networks.end());
    return networks;
}

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.waybar.wifimenu");

    Gtk::Window window;
    
    // Configura o GTK Layer Shell para o Wayland encontrar e posicionar a janela
    GtkWindow* gtk_win = window.gobj();
    gtk_layer_init_for_window(gtk_win);
    gtk_layer_set_layer(gtk_win, GTK_LAYER_SHELL_LAYER_TOP);
    
    // Âncoras: Grudado no Topo e na Direita (Igual ao seu Python)
    gtk_layer_set_anchor(gtk_win, GTK_LAYER_SHELL_EDGE_TOP, true);
    gtk_layer_set_anchor(gtk_win, GTK_LAYER_SHELL_EDGE_RIGHT, true);
    
    // Margens para desgrudar do canto da tela
    gtk_layer_set_margin(gtk_win, GTK_LAYER_SHELL_EDGE_TOP, 25);
    gtk_layer_set_margin(gtk_win, GTK_LAYER_SHELL_EDGE_RIGHT, 20);

    // Estilo CSS
    auto css = Gtk::CssProvider::create();
    css->load_from_data(
        "window { background-color: rgba(30, 30, 46, 0.95); border: 2px solid #ffffff; border-radius: 12px; }"
        "box { padding: 12px; }"
        "label.title { color: #a6adc8; font-weight: bold; font-size: 11px; margin-top: 5px; margin-bottom: 2px; }"
        "button { background: #313244; color: #cdd6f4; border: none; border-radius: 6px; padding: 8px 16px; font-weight: bold; }"
        "button:hover { background: #ffffff; color: #11111b; }"
        ".wifi-list-btn { background: #1e1e2e; font-weight: normal; }"
        ".wifi-list-btn:hover { background: #45475a; color: #ffffff; }"
    );
    auto screen = Gdk::Screen::get_default();
    Gtk::StyleContext::add_provider_for_screen(screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    Gtk::Box box(Gtk::ORIENTATION_VERTICAL, 8);
    bool wifi_active = is_wifi_on();

    std::unique_ptr<Gtk::Button> btn_toggle;
    if (wifi_active) {
        btn_toggle = std::make_unique<Gtk::Button>("󰤭  Desligar Wifi");
    } else {
        btn_toggle = std::make_unique<Gtk::Button>("󰤨  Ligar Wifi");
    }
    btn_toggle->set_halign(Gtk::ALIGN_CENTER);

    btn_toggle->signal_clicked().connect([wifi_active]() {
        if (wifi_active) {
            std::system("/usr/bin/nmcli radio wifi off");
        } else {
            std::system("/usr/bin/nmcli radio wifi on");
        }
        Gtk::Main::quit();
    });
    box.pack_start(*btn_toggle);

    std::vector<std::unique_ptr<Gtk::Button>> net_buttons;

    if (wifi_active) {
        auto lbl_list = Gtk::make_managed<Gtk::Label>("REDES DISPONÍVEIS:");
        lbl_list->get_style_context()->add_class("title");
        lbl_list->set_halign(Gtk::ALIGN_START);
        box.pack_start(*lbl_list);

        auto networks = get_wifi_networks();
        if (!networks.empty()) {
            for (const auto& net : networks) {
                auto net_btn = std::make_unique<Gtk::Button>("  " + net);
                net_btn->get_style_context()->add_class("wifi-list-btn");
                net_btn->set_halign(Gtk::ALIGN_START);
                
                net_btn->signal_clicked().connect([net]() {
                    std::string cmd = "/usr/bin/nmcli device wifi connect '" + net + "' &";
                    std::system(cmd.c_str());
                    Gtk::Main::quit();
                });
                
                box.pack_start(*net_btn);
                net_buttons.push_back(std::move(net_btn));
            }
        } else {
            auto lbl_none = Gtk::make_managed<Gtk::Label>("Nenhuma rede encontrada...");
            lbl_none->set_halign(Gtk::ALIGN_START);
            box.pack_start(*lbl_none);
        }
    }

    // Fecha ao perder o foco
    window.signal_focus_out_event().connect([&window](GdkEventFocus*) {
        Gtk::Main::quit();
        return false;
    });

    window.add(box);
    window.show_all();

    return app->run(window);
}