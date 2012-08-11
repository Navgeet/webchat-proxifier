hcode {
struct login_gui;
void go_gui(struct login_gui *, const char *, int);
}

ccode {
#include <string.h>
#include "err.h"

void go_gui(login_gui *l, const char *img, int len)
{
  gtk_init(0, 0);
  init_login_gui(l, img, len);
  gtk_widget_show_all(l->w);
  gtk_main();
}

}

widget login_gui(const char *img, int l)
{
  { char *in; }

  window w;
  text t;
  box img_box;

  w.global = true;
  t.global = true;

  t.grow = fill;

  w > img_box;
  w >(grow:fill) t;

  t.activate = {
    this->in = strdup(gtk_entry_get_text(GTK_ENTRY(this->t)));
    if (!this->in) OOM();
    gtk_widget_destroy(this->w);
    gtk_main_quit();
  };

  init {
    GtkWidget *im;
    GInputStream *in = g_memory_input_stream_new_from_data(img, l, NULL);
    GdkPixbuf *b = gdk_pixbuf_new_from_stream(in, NULL, NULL);
    if (!b) { printf("bad image\n"); exit(1); }
    g_input_stream_close(in, NULL, NULL);
    im = gtk_image_new_from_pixbuf(b);
    g_object_unref(b);
    gtk_container_add(GTK_CONTAINER(img_box), im);
  }
}
