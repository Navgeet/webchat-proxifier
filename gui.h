/* generated by gtkcooker (http://www.nongnu.org/gtkcooker) */

#ifndef _GUI_H_
#define _GUI_H_



struct login_gui;
void go_gui(struct login_gui *, const char *, int);

typedef struct login_gui {
 char *in; 
  /* GtkWidget* */ void *w;
  /* GtkWidget* */ void *t;
  unsigned long t_activate;
} login_gui;

int init_login_gui(login_gui *this, const char *img, int l);

#endif /* _GUI_H_ */
