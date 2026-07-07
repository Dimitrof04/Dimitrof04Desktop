#!/usr/bin/env lua54
local lgi = require('lgi')
local Gtk = lgi.Gtk
local Gio = lgi.Gio


-- Inicializa o GTK
Gtk.init()

-- Cria a janela do menu
local window = Gtk.Window {
    title = "Menu",
    type = Gtk.WindowType.TOPLEVEL,
    decorated = false, -- Remove as bordas da janela para parecer um menu
    resizable = false,
    default_width = 200,
    default_height = 120,
}

-- Fecha o menu se ele perder o foco (clicar fora dele)
window.on_focus_out_event = function()
    Gtk.main_quit()
    return true
end

-- Caixa vertical para organizar os botões
local box = Gtk.Box { orientation = Gtk.Orientation.VERTICAL, spacing = 6 }

-- Criando os botões do menu
local btn_wifi_on = Gtk.Button { label = "Ligar Wifi" }
local btn_wifi_off = Gtk.Button { label = "Desligar Wifi" }
local btn_settings = Gtk.Button { label = "Configuracoes" }

-- Funções de clique usando comandos do sistema (nmcli para Wi-Fi)
btn_wifi_on.on_clicked = function()
    os.execute("nmcli radio wifi on")
    Gtk.main_quit()
end

btn_wifi_off.on_clicked = function()
    os.execute("nmcli radio wifi off")
    Gtk.main_quit()
end

btn_settings.on_clicked = function()
    os.execute("nm-connection-editor &") -- Abre o gerenciador de redes padrão
    Gtk.main_quit()
end

-- Adiciona os botões na caixa e a caixa na janela
box:add(btn_wifi_on)
box:add(btn_wifi_off)
box:add(btn_settings)
window:add(box)

-- Mostra tudo
window:show_all()
Gtk.main()