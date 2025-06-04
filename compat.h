#ifndef GPPPKILL_COMPAT_H
#define GPPPKILL_COMPAT_H

#include <gtk/gtk.h>

/* Basic compatibility helpers so this GTK1-era code at least compiles with
 * more recent toolkits.  These stubs are very limited and the resulting
 * application will not behave correctly under GTK4 without considerable
 * additional work. */

#if GTK_MAJOR_VERSION < 3
/* map GTK3 functions to GTK2 equivalents */
static inline GtkWidget *gtk_box_new(GtkOrientation orientation, gint spacing) {
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
        return gtk_hbox_new(FALSE, spacing);
    else
        return gtk_vbox_new(FALSE, spacing);
}
static inline GtkWidget *gtk_separator_new(GtkOrientation orientation) {
    return (orientation == GTK_ORIENTATION_HORIZONTAL) ?
        gtk_hseparator_new() : gtk_vseparator_new();
}
#else /* GTK3+ */
#if GTK_MAJOR_VERSION >= 4
#define gtk_window_new(type) gtk_window_new()
#define GTK_CONTAINER(obj) (obj)
static inline void gtk_container_set_border_width(GtkWidget *wid, guint bw) {
    gtk_widget_set_margin_top(wid, bw);
    gtk_widget_set_margin_bottom(wid, bw);
    gtk_widget_set_margin_start(wid, bw);
    gtk_widget_set_margin_end(wid, bw);
}
static inline void gtk_container_add(GtkWidget *container, GtkWidget *child) {
    if (GTK_IS_WINDOW(container)) {
        gtk_window_set_child(GTK_WINDOW(container), child);
    } else if (GTK_IS_BOX(container)) {
        gtk_box_append(GTK_BOX(container), child);
    }
}
static inline void gtk_box_pack_start(GtkBox *box, GtkWidget *child,
                                      gboolean expand, gboolean fill,
                                      guint padding) {
    (void)expand; (void)fill; (void)padding;
    gtk_box_append(box, child);
}
static inline void gtk_widget_destroyed(GtkWidget *widget, GtkWidget **ptr) {
    (void)widget;
    if (ptr)
        *ptr = NULL;
}
#endif /* GTK4 */
#endif /* GTK_MAJOR_VERSION < 3 */

#endif /* GPPPKILL_COMPAT_H */
