#ifndef GPPPKILL_COMPAT_H
#define GPPPKILL_COMPAT_H

#include <gtk/gtk.h>

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
#endif

#endif /* GPPPKILL_COMPAT_H */
