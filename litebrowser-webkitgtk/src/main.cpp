#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>

struct Ui {
  GtkWidget* window{};
  GtkWidget* header{};
  GtkWidget* box{};
  GtkWidget* url_entry{};
  GtkWidget* back_btn{};
  GtkWidget* fwd_btn{};
  GtkWidget* reload_btn{};
  WebKitWebView* view{};
  WebKitUserContentManager* ucm{};
};

static std::string ensure_scheme(const std::string& s) {
  if (s.rfind("http://", 0) == 0 || s.rfind("https://", 0) == 0 || s.rfind("file://", 0) == 0)
    return s;
  // basic heuristic: add https
  return std::string("https://") + s;
}

static void on_load_changed(WebKitWebView* view, WebKitLoadEvent load_event, gpointer user_data) {
  Ui* ui = static_cast<Ui*>(user_data);
  const gchar* uri = webkit_web_view_get_uri(view);
  if (uri && (load_event == WEBKIT_LOAD_COMMITTED || load_event == WEBKIT_LOAD_FINISHED)) {
    gtk_entry_set_text(GTK_ENTRY(ui->url_entry), uri);
  }
}

static gboolean on_key_press(GtkWidget*, GdkEventKey* e, gpointer user_data) {
  Ui* ui = static_cast<Ui*>(user_data);
  // Ctrl+L => focus URL
  if ((e->state & GDK_CONTROL_MASK) && (e->keyval == GDK_KEY_l || e->keyval == GDK_KEY_L)) {
    gtk_widget_grab_focus(ui->url_entry);
    gtk_editable_select_region(GTK_EDITABLE(ui->url_entry), 0, -1);
    return TRUE;
  }
  // Alt+Left / Alt+Right
  if ((e->state & GDK_MOD1_MASK) && e->keyval == GDK_KEY_Left) {
    if (webkit_web_view_can_go_back(ui->view)) webkit_web_view_go_back(ui->view);
    return TRUE;
  }
  if ((e->state & GDK_MOD1_MASK) && e->keyval == GDK_KEY_Right) {
    if (webkit_web_view_can_go_forward(ui->view)) webkit_web_view_go_forward(ui->view);
    return TRUE;
  }
  // Ctrl+R reload
  if ((e->state & GDK_CONTROL_MASK) && (e->keyval == GDK_KEY_r || e->keyval == GDK_KEY_R)) {
    webkit_web_view_reload(ui->view);
    return TRUE;
  }
  return FALSE;
}

static void on_entry_activate(GtkEntry* entry, gpointer user_data) {
  Ui* ui = static_cast<Ui*>(user_data);
  const char* text = gtk_entry_get_text(entry);
  std::string url = ensure_scheme(text ? text : "");
  webkit_web_view_load_uri(ui->view, url.c_str());
}

static gboolean on_decide_policy(WebKitWebView*, WebKitPolicyDecision* decision, WebKitPolicyDecisionType type, gpointer) {
  if (type == WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION) {
    webkit_policy_decision_ignore(decision);
    return TRUE;
  }
  return FALSE;
}

static void on_download_started(WebKitWebContext*, WebKitDownload* download, gpointer) {
  // Destination in ~/Downloads (or env override)
  const gchar* suggested = webkit_download_get_suggested_filename(download);
  const char* custom = g_getenv("LITEBROWSER_DOWNLOAD_DIR");
  const char* home_dl = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
  std::filesystem::path dir = custom ? custom : (home_dl ? home_dl : g_get_home_dir());
  std::filesystem::create_directories(dir);
  std::filesystem::path out = dir / (suggested ? suggested : "download.bin");
  std::string dest = std::string("file://") + out.string();
  webkit_download_set_destination(download, dest.c_str());
  g_signal_connect(download, "finished", G_CALLBACK(+[] (WebKitDownload* d, gpointer) {
    g_message("Download finished: %s", webkit_download_get_destination(d));
  }), nullptr);
}

static void maybe_load_content_filter(Ui* ui) {
  // If a compiled filter named "adblock" exists in store_dir, load it.
  std::filesystem::path store_dir = std::filesystem::path("resources") / "filters" / "store";
  if (!std::filesystem::exists(store_dir)) return;

  GError* error = nullptr;
  WebKitUserContentFilterStore* store =
      webkit_user_content_filter_store_new(store_dir.string().c_str());
  webkit_user_content_filter_store_load(store, "adblock", nullptr,
    +[] (GObject* source, GAsyncResult* res, gpointer user_data) {
      Ui* ui = static_cast<Ui*>(user_data);
      GError* err = nullptr;
      WebKitUserContentFilter* filter =
        webkit_user_content_filter_store_load_finish(WEBKIT_USER_CONTENT_FILTER_STORE(source), res, &err);
      if (err) {
        g_message("Content filter not loaded: %s", err->message);
        g_error_free(err);
        return;
      }
      webkit_user_content_manager_add_filter(ui->ucm, filter);
      g_message("Adblock content filter loaded.");
    }, ui);
}

int main(int argc, char** argv) {
  gtk_init(&argc, &argv);

  Ui ui{};

  ui.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(ui.window), 1100, 740);
  gtk_window_set_title(GTK_WINDOW(ui.window), "LiteBrowser");

  g_signal_connect(ui.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(ui.window, "key-press-event", G_CALLBACK(on_key_press), &ui);

  ui.header = gtk_header_bar_new();
  gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(ui.header), TRUE);
  gtk_header_bar_set_title(GTK_HEADER_BAR(ui.header), "LiteBrowser");
  gtk_window_set_titlebar(GTK_WINDOW(ui.window), ui.header);

  ui.box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(ui.window), ui.box);

  // Toolbar: back, fwd, reload, url
  GtkWidget* toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_set_homogeneous(GTK_BOX(toolbar), FALSE);
  gtk_box_pack_start(GTK_BOX(ui.box), toolbar, FALSE, FALSE, 4);

  ui.back_btn = gtk_button_new_from_icon_name("go-previous");
  ui.fwd_btn  = gtk_button_new_from_icon_name("go-next");
  ui.reload_btn = gtk_button_new_from_icon_name("view-refresh");
  ui.url_entry = gtk_entry_new();

  gtk_box_pack_start(GTK_BOX(toolbar), ui.back_btn, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(toolbar), ui.fwd_btn, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(toolbar), ui.reload_btn, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(toolbar), ui.url_entry, TRUE, TRUE, 0);

  // WebView with its own content manager
  ui.ucm = webkit_user_content_manager_new();
  ui.view = WEBKIT_WEB_VIEW(webkit_web_view_new_with_user_content_manager(ui.ucm));

  // Settings
  WebKitSettings* s = webkit_settings_new_with_settings(
      "enable-developer-extras", TRUE,
      "enable-javascript", TRUE,
      "enable-accelerated-2d-canvas", TRUE,
      "enable-webgl", TRUE,
      NULL);
  webkit_web_view_set_settings(ui.view, s);

  // Hook up signals
  g_signal_connect(ui.view, "load-changed", G_CALLBACK(on_load_changed), &ui);
  g_signal_connect(ui.view, "decide-policy", G_CALLBACK(on_decide_policy), NULL);
  g_signal_connect(ui.url_entry, "activate", G_CALLBACK(on_entry_activate), &ui);

  // Button clicks
  g_signal_connect(ui.back_btn, "clicked", G_CALLBACK(+[] (GtkButton*, gpointer u) {
    Ui* ui = static_cast<Ui*>(u);
    if (webkit_web_view_can_go_back(ui->view)) webkit_web_view_go_back(ui->view);
  }), &ui);
  g_signal_connect(ui.fwd_btn, "clicked", G_CALLBACK(+[] (GtkButton*, gpointer u) {
    Ui* ui = static_cast<Ui*>(u);
    if (webkit_web_view_can_go_forward(ui->view)) webkit_web_view_go_forward(ui->view);
  }), &ui);
  g_signal_connect(ui.reload_btn, "clicked", G_CALLBACK(+[] (GtkButton*, gpointer u) {
    Ui* ui = static_cast<Ui*>(u);
    webkit_web_view_reload(ui->view);
  }), &ui);

  // Downloads
  WebKitWebContext* ctx = webkit_web_view_get_context(ui.view);
  g_signal_connect(ctx, "download-started", G_CALLBACK(on_download_started), NULL);

  // Try to load compiled content filter if present
  maybe_load_content_filter(&ui);

  gtk_box_pack_start(GTK_BOX(ui.box), GTK_WIDGET(ui.view), TRUE, TRUE, 0);

  gtk_widget_show_all(ui.window);

  const char* start = (argc > 1) ? argv[1] : "https://example.com";
  webkit_web_view_load_uri(ui.view, start);

  gtk_main();
  return 0;
}
