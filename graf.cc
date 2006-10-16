/*
 *  gpppkill. X11/GTK+ program that kill pppd if it not recive a minimal amount 
 *  of bytes during certain time. It also plot the amount bytes/seg recived.
 *  Copyright (C) 1998  Oliver Schulze L.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *  
 *  You can reach the author at: 
 *    oliver@pla.net.py
 *
 *  gpppkill Home Page:
 *    http://www.pla.net.py/home/oliver/gpppkill/
 *  
 *  A copy of the GNU General Public License is included with this program.
 */
#include "graf.h"

/*------------------------------------------------------------------------------
 */
graf::graf(class gpppkill *g)
{
	gpppk = g;

	pixmap = NULL;
	draw = NULL;
	frame_graf = NULL;
	pix = NULL;
	barras_in = NULL;
	barras_out = NULL;

	x=0,							//posicion a dibujar la sigte barra
	cambio = (-1),		//bandera para cambiar de escala. Ver comentario de la funcion.
	cambiar_escala = 0,	//bandera para ver si redibujo el grafico con una nueva escala.
	linea_ancho = DEF_LINEA_ANCHO,	//se fue a gpppkill::linea_ancho //antes era DRAW_LINEA_ANCHO
	factor_escala_minima = FACTOR_ESCALA_MINIMA,	//(gpppk->DRAW_ALTO*10) == 1000 bytes
	factor_escala_maxima = FACTOR_ESCALA_MAXIMA,	//escala maxima (gpppk->DRAW_ALTO*100) == 10000 bytes
	incremento_escala = (gpppk->DRAW_ALTO * factor_escala_minima),	//es la diferencia entre escalas
	escala = (gpppk->DRAW_ALTO * factor_escala_minima);	//escala minima. 

	//colores
	rojo = NULL; 
	verde = NULL;
	amarillo = NULL;
	color_linea = NULL;

	//g_print("graf::graf(): gpppk->DRAW_ALTO: %d\n", gpppk->DRAW_ALTO);
}

/*------------------------------------------------------------------------------
 */
graf::~graf()
{
	if(rojo)
		free(rojo);

	if(verde)
		free(verde);

	if(amarillo)
		free(amarillo);

	if(color_linea)
		free(color_linea);
}

/*------------------------------------------------------------------------------
 */
void graf::crear_draw()
{
  // --- Crear drawing_area ---
  draw = gtk_drawing_area_new();

  // Conectar drawing_area a su callbacks
  gtk_signal_connect(	GTK_OBJECT(draw), 
  										"expose_event",
		      						(GtkSignalFunc) graf_expose_event_callback, 
		      						this);
  gtk_signal_connect( GTK_OBJECT(draw),  
  										"configure_event",
		      						(GtkSignalFunc) graf_configure_event_callback, 
		      						this);

	//Propiedadees de drawing_area
  gtk_widget_set_events(draw, GDK_EXPOSURE_MASK);
  gtk_drawing_area_size (GTK_DRAWING_AREA (draw), gpppk->DRAW_ANCHO, gpppk->DRAW_ALTO);
}

/*------------------------------------------------------------------------------
 *	Llamar una sola vez a esta funcion.
 */
void graf::graf_init(void)
{
	cant_barras = ( (int)(gpppk->DRAW_ANCHO / linea_ancho) );

	// se le suma +1 porque se neceista tener la ultima barra borrada.
	barras_in  = new listac( (int)(gpppk->DRAW_ANCHO / linea_ancho) + 1 );

	// se le suma +1 porque se neceista tener la ultima barra borrada.
	barras_out = new listac( (int)(gpppk->DRAW_ANCHO / linea_ancho) + 1 );

	color_init();
}

/*------------------------------------------------------------------------------
 *
 */
void graf::color_init(void)
{
  //color->red = red * (65535/255);
  //color->green = green * (65535/255);
  //color->blue = blue * (65535/255);
  //color->pixel = (gulong)(red*65536 + green*256 + blue);

  rojo = (GdkColor *)malloc(sizeof(GdkColor));
	//gdk_color_parse("red", rojo);
  rojo->red = 255 * (65535/255);
  rojo->green = 165 * (65535/255);
  rojo->blue = 0;
  rojo->pixel = (gulong)(255*65536 + 0 + 0);

  verde = (GdkColor *)malloc(sizeof(GdkColor));
  verde->red = 41 * (65535/255);
  verde->green = 137 * (65535/255);
  verde->blue = 82 * (65535/255);   
  verde->pixel = (gulong)(0 + 255*256 + 0);

  amarillo = (GdkColor *)malloc(sizeof(GdkColor));
  amarillo->red = 255 * (65535/255);
  amarillo->green = 255 * (65535/255);
  amarillo->blue = 0;
  amarillo->pixel = (gulong)(255*65536 +255*256 +0);

  color_linea = (GdkColor *)malloc(sizeof(GdkColor));
  color_linea->red = 102 * (65535/255);
  color_linea->green = 102 * (65535/255);
  color_linea->blue = 102 * (65535/255);
  color_linea->pixel = (gulong)(255*65536 +255*256 +0);

 //Colores para el drawing area.
	gc = gdk_gc_new(draw->window);
  gdk_color_alloc(gtk_widget_get_colormap(draw), rojo);
  gdk_color_alloc(gtk_widget_get_colormap(draw), verde);
  gdk_color_alloc(gtk_widget_get_colormap(draw), amarillo);
  gdk_color_alloc(gtk_widget_get_colormap(draw), color_linea);
}

/*------------------------------------------------------------------------------
 *	Dibuja el grafico de barras.
 *	escala:
 *		La escala maxima es (gpppk->DRAW_ALTO * factor_escala_maxima) 
 *		La escala minima es (gpppk->DRAW_ALTO * factor_escala_minima)
 *		La escala se cambia cuando desaparece o aparece una barra con altura_in maxima.
 *			Cada vez que sucede esto se llama a barras_in->maximo_saltando(1).
 *		La escala se aumenta(o disminuye) al sigte multiplo de 1000 mayor que el
 *			gpppk->stats.in actual.
 *	variable cambio:
 *		cambio es un contador de la posicion en que esta la barra de mayor altura_in.
 *		Si (cambio == x) siendo (x>0), entonces la barra de mayor altura_in esta a x barras empezando de la izquierda.
 *		Si (cambio == 0) entonces la barra de mayor altura_in ya salio del grafico. Buscar una nueva escala.
 *		Si (cambio == 0) encontces debo disminuir la escala. La nueva escala la obtengo con ayuda de listac.maximo_saltando(1)
 *		Si (cambio == -1) entonces escala==1000 y no necesito buscar una escala menor.
 *	Si ((gpppk->stats.in) > escala) entonces debo aumentar la escala.
 *	Variable 'cambiar_escala':
 *		Si == 1 se debe cambiar la escala.
 *		Si == 2 ya se cambio la escala. Esto es para que no se vuelva a redibujar.
 *		Si == 0 no hace falta cambiar la escala.
 *	cant_barras:
 *		cantidad de baras que tiene el grafico. 
 *		Aunque no todas entren completamente en el grafico.
 *
 *
 *
 *	argument:
 *		- pppkill::pppstats_actual
 */
void graf::dibujar(void)
{

	//------------- debug/test ---------------------
/*
	static int ban = 2, 
						 rand = 500,
						 pico = 0;

	if(rand > ((gpppk->DRAW_ALTO*factor_escala_maxima)-incremento_escala) )
		ban=0;
	if(rand < (gpppk->DRAW_ALTO*factor_escala_minima))
		ban=1;

	if(!pico)
		gpppk->stats.in = 20000;
	else 
		gpppk->stats.in = 3900;
	pico++;
	if(pico>105) pico=0;
*/

/*// debug option 2
	if(ban) {
		rand = rand + incremento_escala;
		gpppk->stats.in = rand;
		gpppk->stats.out = 1000;
	}
	else {
		rand = rand - incremento_escala;
		gpppk->stats.in = rand;
		gpppk->stats.out = 1000;
	}
*/

	//gpppk->stats.in = rand;
	//^^^^^^^^^^^^^^ debug/test ^^^^^^^^^^^^^^^^^^^^^^

	//agregar el valor actual
	barras_in->agregar_al_final((int)gpppk->stats.in);
	barras_out->agregar_al_final((int)gpppk->stats.out);

	//--- Calcular y cambiar la escala --------------------------------------
	if(((int)gpppk->stats.in >= escala) || ((int)gpppk->stats.out >= escala)) {
		//si no estoy en la maxima escala
		if( escala != (gpppk->DRAW_ALTO * factor_escala_maxima) )
			cambio = cant_barras;	//cambiar de escala.
	}
	else {
		if(cambio>1)
			cambio--;	//esperar hasta que desaparecesca el pico.
	}
	//if(barras_in->usados() == barras_in->capacidad()) {
	//if(barras_in->esta_llena()) {
		//la lista esta llena, liberar un nodo.
 		//barras_in->borrar_al_inicio();
 	//}

	//agregar el valor actual
	//barras_in->agregar_al_final((int)gpppk->stats.in);
	//barras_out->agregar_al_final((int)gpppk->stats.out);

	//cambio la escala cuando desaparece o aparece una barra con altura_in maxima
	if((cambio==1) || (cambio == cant_barras)) {
		// Ver si se va a mover el grafico. 
		// Si no se tiene que mover, entonces se debe incluir el primer elemento 
		// de la listac para hallar el maximo.
		if( x>=(gpppk->DRAW_ANCHO) ) {
			max_in = barras_in->maximo_saltando(1);
			max_out = barras_out->maximo_saltando(1);
		}
		else { // incluir el primer elemento de la lista
			max_in = barras_in->maximo_saltando(0);
			max_out = barras_out->maximo_saltando(0);
		}
		
		//la escala se aumenta(o disminuye) al sigte multiplo de 1000 mayor que gpppk->stats.in
		//redondeo a un factor de incremento_escala. ej: 3012 -> 3000
		if(max_in >= max_out) 
			escala = (max_in / incremento_escala) * incremento_escala;
		else
			escala = (max_out / incremento_escala) * incremento_escala;	
		
		escala = escala  + incremento_escala;	//aumento la escala
		//cout << "cambio de escala-> escala: " << escala << " cambio: "<<cambio << endl;
		cambiar_escala = 1;
		//printf("barras_in->maximo_saltando(1)\n");
		//si llegue a la minima escala, no buscar escala menor.
		if(escala == (gpppk->DRAW_ALTO * factor_escala_minima))	
			cambio = -1;
	}

	//cout << "escala: " << escala <<" maximo_saltando(1): " <<barras_in->maximo_saltando(1) <<" maximo_saltando(0): " <<barras_in->maximo_saltando(0)<< endl;
	//printf("escala:%d in:%d cambio:%d \n", escala, (int)gpppk->stats.in, cambio);
	//barras_in->imprimir();
	//cout << "usados:" << barras_in->usados() << endl;
	//cout << "x: " << x << endl;
	//cout << "escala: " << escala << endl;
	//cout << "barras_in->maximo: " << barras_in->maximo_saltando(1) << endl;
	//cout << endl;
	//^^^ FIN Calcular y cambiar la escala ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/*
 *	if (cambiar_escala)
 *		borrar_grafico
 *		while(ciclar todas las barras desde la barra borrada.) {
 *			if(no entra la proxima barra)
 *				mover grafico a la izquierda.
 *			dibujar la barra.
 *		}
 */

	if(cambiar_escala) {
		//aqui cambio la escala, dibujo todo de nuevo y dibujo la barra actual.

		//pinto todo de blanco pixmap
		borrar();

		ini_in = barras_in->getinicio();
		ini_out = barras_out->getinicio();
		fin = barras_in->getfinal();	//barras.[in,out] tienen la misma longitud.
		x = 0;
		//ciclar cada barra de las listas barras_[in.out] 
		while(ini_in != fin) {
			//g_print("(x+linea_ancho) > (gpppk->DRAW_ANCHO-1): (%d > %d)\n", (x+linea_ancho), (gpppk->DRAW_ANCHO-1));

			// ver si hay lugar para la sigte barra.
			if( (x+linea_ancho) > (gpppk->DRAW_ANCHO-1) ) {//copiado de graf.cc@401
				//aqui SOLO muevo el grafico a la izquierda.
				copiar_pixmap();
				borrar();
				//gdk_gc_set_foreground(gc, verde);
				gdk_gc_set_foreground(gc, color_linea);
				mover();
				//desminuir x para dibujar la sigte barra
		 		x = gpppk->DRAW_ANCHO - linea_ancho;

				//cerr << "despues de cambiar escala. x: " << x << endl;
			}//	if( (x+linea_ancho) > (gpppk->DRAW_ANCHO-1) ) 

			//dijbujar la barra en el draw(pixmap)

			//obtener la altura_in del nodo de 'barras_in'
			if(ini_in->valor)
				altura_in = (ini_in->valor / (escala/gpppk->DRAW_ALTO));
			else
				altura_in = 0;

			if(ini_out->valor)
				altura_out = (ini_out->valor / (escala/gpppk->DRAW_ALTO));
			else
				altura_out = 0;

			//dijbujar la nueva barra en el draw.
			if(altura_in > altura_out) {	//si la barra verde es mas alta
				linea_in((gpppk->DRAW_ALTO-altura_in), altura_in - altura_out);
				linea_inout((gpppk->DRAW_ALTO-altura_out), altura_out);
			}
			else { 
				if(altura_in < altura_out) {//si la barra roja es mas alta
					linea_out((gpppk->DRAW_ALTO-altura_out), altura_out - altura_in);
					linea_inout((gpppk->DRAW_ALTO-altura_in), altura_in);
				}
				else {	//si las barras roja y verde son iguales
					linea_inout((gpppk->DRAW_ALTO-altura_in), altura_in);
				}
			}

			//Actualizar el contador de posicion horizontal, osea 'x'
			x = x + linea_ancho;

			//ir al sigte nodo
			ini_in = ini_in->sigte;
			ini_out = ini_out->sigte;
		};
		cambiar_escala = 0;	//ya cambie la escala
		//cerr << "despues de cambiar escala. x: " << x << endl;
	}
	else {
		//Si la sigte barra a dibujar no va a entrar en el grafico, entonces 
		//	mover lo que ya esta dibujado hacia la izquierda
		if( (x+linea_ancho) > (gpppk->DRAW_ANCHO-1) ) {
			//aqui SOLO muevo el grafico a la izquierda.
			copiar_pixmap();
			borrar();
			gdk_gc_set_foreground(gc, verde);
			mover();

			//desminuir x para dibujar la sigte barra
	 		x = gpppk->DRAW_ANCHO - linea_ancho;
		}//	if( (x+linea_ancho) > (gpppk->DRAW_ANCHO-1) ) 

		// --- dibujar la barra actual. No hubo cambio de escala. ---------------
		if(gpppk->stats.in)
			altura_in = (gpppk->stats.in / (escala/gpppk->DRAW_ALTO));
		else
			altura_in = 0;

		if(gpppk->stats.out)
			altura_out = (gpppk->stats.out / (escala/gpppk->DRAW_ALTO));
		else
			altura_out = 0;

		//dijbujar la nueva barra en el draw.
		if(altura_in > altura_out) {	//si la barra verde es mas alta
			linea_in((gpppk->DRAW_ALTO-altura_in), altura_in - altura_out);
			linea_inout((gpppk->DRAW_ALTO-altura_out), altura_out);
		}
		else { 
			if(altura_in < altura_out) {//si la barra roja es mas alta
				linea_out((gpppk->DRAW_ALTO-altura_out), altura_out - altura_in);
				linea_inout((gpppk->DRAW_ALTO-altura_in), altura_in);
			}
			else {	//si las barras roja y verde son iguales
				linea_inout((gpppk->DRAW_ALTO-altura_in), altura_in);
			}
		}

		//Actualizar el contador de posicion horizontal, osea 'x'
		x = x + linea_ancho;

		//cerr << "despues de mantener escala. x: " << x << endl;
	}//	if(cambiar_escala)

	// --- Dibujar las lineas de la escala ---------------------
	gdk_gc_set_foreground(gc, color_linea);
	altura_in = (int)ceil( (float)gpppk->DRAW_ALTO / ((float)escala/(float)incremento_escala));
/*{//debug
	double es, in;
	es = escala;
	in = incremento_escala;
	cout << endl;
	cout << "int  numero de lineas: " << (float)(escala/incremento_escala) << endl;
	cout << "ceil numero de lineas: " << ceil(es/in) << endl;
	cout << "int (separacion de lineas)  : " << altura_in << endl;
	cout << "ceil(separacion de lineas)  : " << ceil((float)(gpppk->DRAW_ALTO / (es/in))) << endl;
	cout << "ceil(separacion de lineas)  : " << ceil(gpppk->DRAW_ALTO / (es/in)) << endl;
	//altura_in = (int)ceil((float)(gpppk->DRAW_ALTO / (es/in)));
}*/
	/*
	 *	(escala/incremento_escala):  es la cantidad maxima de lineas de escala,
	 *		esta divicion da siempre int porque escala es un nultiple de incremento_escala
	 *	i                         :  es el numero de lineas de escala.
	 */
	for(i=1; i <= (escala/incremento_escala); i++) {
		gdk_draw_line( pixmap,
												gc,
												0 				  , ( (gpppk->DRAW_ALTO-1) - (i*altura_in) ),		//(x1, y1)
												gpppk->DRAW_ANCHO-1, ( (gpppk->DRAW_ALTO-1) - (i*altura_in) ) );	//(x2, y2)
		//cout << i*(gpppk->DRAW_ALTO/altura_in)-1 << ", " ;
	}
	//cout << endl;
/*{//debug
	gdk_gc_set_foreground(gc, rojo);
	gdk_draw_line( pixmap,
											gc,
											gpppk->DRAW_ANCHO-1, (gpppk->DRAW_ALTO-1),		//(x1, y1)
											gpppk->DRAW_ANCHO-1, (gpppk->DRAW_ALTO-1) );	//(x2, y2)
}*/

	//Pedirle a X que actualize todo el drawing_area
  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = gpppk->DRAW_ANCHO;
  update_rect.height = gpppk->DRAW_ALTO;

	//dibujar todas las barras_in del grafico(bar plot)
	if(GTK_WIDGET_VISIBLE(frame_graf))
	  gtk_widget_draw (draw, &update_rect);

	// debug
	//cerr << "graf::dibujar() x: "<<x << " escala: "<<escala << endl;
	//g_print("cant de barras actuales: %d\n", x/linea_ancho);
	//g_print("cant:%d barras_in->capacidad():%d\n", cant_barras, barras_in->capacidad());
	//g_print("linea_ancho: %d\n", linea_ancho);
}

/*------------------------------------------------------------------------------
 *	Dibujar de blanco el drawing area.
 *	Resetea las variables que se usan para dibujar.
 */
void graf::dibujar_reset(void)
{
	GdkRectangle update_rect;

	if(pixmap == NULL)
		return;

	x = 0;
	cambio = (-1);
	cambiar_escala = 0;
	escala = (gpppk->DRAW_ALTO * factor_escala_minima);	//escala minima. 

	//barras_in->vaciar();
	//barras_out->vaciar();
	if(barras_in) delete barras_in;
	barras_in  = new listac( (int)(gpppk->DRAW_ANCHO / linea_ancho) + 1 );

	// se le suma +1 porque se neceista tener la ultima barra borrada.
	if(barras_out) delete barras_out;
	barras_out = new listac( (int)(gpppk->DRAW_ANCHO / linea_ancho) + 1 );
			
	//pinto todo de blanco el pixmap
  gdk_draw_rectangle (pix,
		      						draw->style->white_gc,
		      						TRUE,
		      						0, 0,
		      						gpppk->DRAW_ANCHO, gpppk->DRAW_ALTO);

	//pinto todo de blanco el pixmap
	borrar();
			      						
  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = gpppk->DRAW_ANCHO;
  update_rect.height = gpppk->DRAW_ALTO;

	//actualizar todo el pixmap
	if(GTK_WIDGET_VISIBLE(frame_graf))
	  gtk_widget_draw (draw, &update_rect);

	//g_print("graf::dibujar_reset\n");
}

/*------------------------------------------------------------------------------
 *	Pinta todo el grafico de blanco
 */
void graf::borrar(void)
{
	//pinto todo de blanco pixmap
  gdk_draw_rectangle (pixmap,
		      						draw->style->white_gc,
		      						TRUE,
		      						0, 0,
		      						gpppk->DRAW_ANCHO, gpppk->DRAW_ALTO);

}

/*------------------------------------------------------------------------------
 *	copio lo que hay en this->pixmap a this->pix
 */
void graf::copiar_pixmap(void)
{
	//copio pixmap a pix.
	gdk_draw_pixmap(pix,
									draw->style->white_gc,
									pixmap,
									0, 0,
									0, 0,
									gpppk->DRAW_ANCHO, gpppk->DRAW_ALTO);
}

/*------------------------------------------------------------------------------
 *	Observacion 1:	
 *		Copio pix a pixmap, pero dejando espacio para que entre perfectamente
 *			una barra mas a la derecha.
 *		Hasta ahora el espacio que tengo para dibujar la barra actual es: (gpppk->DRAW_ANCHO-x)
 *		Al copiar, copiar desde "lo que me falta para completar una barra", 
 *			"lo que falta" es igual a (El ancho de la barra) - (lo que tengo),
 *			osea                    = (linea_ancho)     - (gpppk->DRAW_ANCHO-x)
 */
void graf::mover(void)
{
	// copiar pix a pixmap
	gdk_draw_pixmap(pixmap,
									gc,
									pix,
									(linea_ancho)-(gpppk->DRAW_ANCHO-x), 0,
									0, 0,
									x, gpppk->DRAW_ALTO);
}

/*------------------------------------------------------------------------------
 */
void graf::linea_in(int pos_y, int alto)
{
	gdk_gc_set_foreground(gc, verde);
	gdk_draw_rectangle( pixmap,
											gc,
											TRUE,
											x 				 , pos_y, //(x, y)
											linea_ancho, alto);//ancho, alto
}

/*------------------------------------------------------------------------------
 */
void graf::linea_out(int pos_y, int alto)
{
	gdk_gc_set_foreground(gc, rojo);
	gdk_draw_rectangle( pixmap,
											gc,
											TRUE,
											x 				 , pos_y, //(x, y)
											linea_ancho, alto);//ancho, alto
}

/*------------------------------------------------------------------------------
 */
void graf::linea_inout(int pos_y, int alto)
{
	gdk_gc_set_foreground(gc, amarillo);
	gdk_draw_rectangle( pixmap,
											gc,
											TRUE,
											x 				 , pos_y, //(x, y)
											linea_ancho, alto);//ancho, alto
}

// --- get funtions
//------------------------------------------------------------------------------
/*
GdkPixmap *graf::getpixmap(void)
{
	return (GdkPixmap *)pixmap;
}
*/
//------------------------------------------------------------------------------
GtkWidget *graf::getdraw(void)
{
	return GTK_WIDGET(draw);
}

// --- set funtions
//------------------------------------------------------------------------------
/*
void graf::setpixmap(GdkPixmap *pix)
{
	pixmap = (GdkPixmap *)pix;
}
*/
//------------------------------------------------------------------------------
void graf::setlinea_ancho(int la)
{
	linea_ancho = la;
}

//------------------------------------------------------------------------------
void graf::setframe_graf(GtkWidget *f)
{
	frame_graf = f;
}

// ### X Windows events ########################################################

/*------------------------------------------------------------------------------
 *	Repinta la ventana. Solo repinta la parte necesaria.
 */
gint graf_expose_event_callback (GtkWidget *widget, GdkEventExpose *event, graf *grafico)
{
	gdk_draw_pixmap(widget->window,
		  						widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  		  			grafico->pixmap,
		  		  			event->area.x, event->area.y,
		  		  			event->area.x, event->area.y,
		  		  			event->area.width, event->area.height);

  return FALSE;
}

/*------------------------------------------------------------------------------
 *	Se llama al crear la ventana y cada vez que se hace resize
 */
gint graf_configure_event_callback(GtkWidget *widget, GdkEventConfigure *event, graf *grafico)
{	
	GdkPixmap *tmp_pix;

	event = event;	//odio los warnings de unused variable
	tmp_pix = grafico->pixmap;
	if(tmp_pix == NULL) {
		//crear el pixmap para dibujar el grafico en el.
	  tmp_pix = (GdkPixmap *)gdk_pixmap_new(widget->window,
				    													widget->allocation.width,
				  														widget->allocation.height,
				  														-1);
		if(tmp_pix == NULL) {	//si no lo pude crear
			cerr << "gpppkill_configure_event_callback: no hay memoria!" << endl;
			return FALSE;
		}
		grafico->pixmap =(GdkPixmap *)tmp_pix;	//guardar en graf::pixmap

		//borrar todo el pixmap( pintar de blanco)
	  gdk_draw_rectangle (tmp_pix,
			      						widget->style->white_gc,
			      						TRUE,
			      						0, 0,
			      						widget->allocation.width,
			      						widget->allocation.height);

		//crear graf::pix
		if(grafico->pix == NULL) {	//si no cree aun pix
			grafico->pix = (GdkPixmap *)gdk_pixmap_new(widget->window,
				    													widget->allocation.width,
				  														widget->allocation.height,
				  														-1);
			if(grafico->pix == NULL) {
				cerr << "gpppkill_configure_event_callback(): no hay memoria!" << endl;
				return FALSE;
			}
		}
		// todo salio bien, actualizar dimension
		grafico->gpppk->DRAW_ANCHO = widget->allocation.width;
		grafico->gpppk->DRAW_ALTO  = widget->allocation.height;
	}	
	//g_print("graf_configure_event_callback\n");
	//g_print("\tAllocation:       w:%d x h:%d\n", widget->allocation.width, widget->allocation.height);
	//g_print("\tEvent x:%d, y:%d  h:%d x h:%d\n", event->x, event->y, 
	//																						 event->width, event->height);

  return TRUE;
}
