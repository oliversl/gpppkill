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
#include "listac.h"

nodoc::nodoc()
{
	valor = 0;
}
/*------------------------------------------------------------------------------
 *	Lista circular de tamanho(elemntos) fijo.
 * -------------------------------------------
 *	La longitud de la lista circular es listac::longitud.
 *	Pero la cantidad de elementos que la lista puesde guardar es:
 *		(longitud - 1)
 *	La lista siempre tiene (longitud-1) elementos. La cantidad de los 
 *		elementos	NO varia.
 *	Un elemento de la lista no se usa para poder determinar el 
 *		estado de la lista: vacia, llena y con elementos libres.
 *	Condiciones:
 *		inicio == final               --> vacia
 *		inicio == final->sigte->sigte --> llena
 *		final apunta siempre a        --> ultimo_elemento->sigte
 *		inicio->sigte == final        --> un solo elemento en la lista
 *
 */
listac::listac(int size)
{
	int i;
	nodoc *nodoaux,
				*nodonuevo;

	longitud = size + 1;

	//crear el primer elemento
	nodoaux = NULL;
	nodoaux = new nodoc();
	if(nodoaux == NULL) {
		fprintf(stderr, "listac::listac()\nNo memory for new nodoc()!\n");
		exit(0);
	}
	nodoaux->valor=0;
	inicio = nodoaux;
	final = inicio;	//condicion lista vacia

	//crear los demas elementos
	for(i=0; i<(longitud-1); i++) {
		nodonuevo = NULL;
		nodonuevo = new nodoc();
		if(nodonuevo == NULL) {
			fprintf(stderr, "listac::listac()\nNo memory for new nodoc()!\n");
			exit(0);
		}
		nodoaux->sigte = nodonuevo;
		nodonuevo->valor = 0;
		nodoaux = nodonuevo;
	}

	//Hacer la lista circular. El ultimo nodo apunta a primero.
	nodoaux->sigte = inicio;	
}

/*------------------------------------------------------------------------------
 *	Destructor de listac.
 *	Libera todos los nodos de la lista.
 */
listac::~listac()
{
	//cout << "Destructor listac..." << endl;
	borrar();
}

/*------------------------------------------------------------------------------
 *	Libera todos los nodos de la lista.
 *	Aqui SI se libera memoria.
 */
void listac::borrar()
{
	int i;
	nodoc *aborrar, *tmp2;

	aborrar = inicio;
	for(i=0; i<longitud; i++) {
		tmp2 = aborrar->sigte;
		delete aborrar;
		aborrar = tmp2;
	}
}

/*------------------------------------------------------------------------------
 *	Coloca la lista en situacion "vacia".
 *	Aqui NO se libera memoria.
 */
void listac::vaciar()
{
	nodoc *nodo;

	nodo = inicio;
	while( nodo != final ) {
		nodo->valor = 0;
		nodo = nodo->sigte;
	};
}

/*------------------------------------------------------------------------------
 *	return:
 *		0: -> no esta llena
 *		1: -> si esta llena
 */
int listac::esta_llena()
{
	if(final->sigte == inicio)
		return 1;
	else
		return 0;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0: -> no esta vacia
 *		1: -> si esta vacia
 */
int listac::esta_vacia()
{
	if(inicio == final)
		return 1;
	else
		return 0;
}

/*------------------------------------------------------------------------------
 *	Desocupa el primer elemento de la lista
 *	Avanza inicio al sigte elemento.
 */
void listac::borrar_al_inicio(void)
{
	if(esta_vacia())	//si la lista esta vacia
		return ;

	inicio->valor = 0;
	inicio = inicio->sigte;
}

/*------------------------------------------------------------------------------
 *	Al agregar, siempre se agrega en final y luego se aumenta final.
 *	Si la lista esta llena, se borra_al_inicio() y luego se agrega.
 */
void listac::agregar_al_final(int val)
{
	if(esta_llena())	//si la lista esta llena
		borrar_al_inicio();
		
	final->valor = val;
	final = final->sigte;
}

/*------------------------------------------------------------------------------
 *	Retorna el maximo valor(nodoc::valor) de entre todos los nodos de la lista.
 *	return:
 *		0: todos los valores son cero o la lista esta vacia.
 *	 >0: el mayor valor de la lista
 */
int listac::maximo(void)
{
	int max;
	nodoc *tmp;

	max = 0;
	tmp = inicio;
	while( tmp != final ) {
		if(tmp->valor > max)
			max = tmp->valor;
		tmp = tmp->sigte;
	};

	return max;	
}

/*------------------------------------------------------------------------------
 *	Retorna el maximo valor(nodoc::valor) de entre todos los nodos de la lista,
 *	saltando los primeros 'saltar' elementos.
 *	return:
 *		0: todos los valores son cero o la lista esta vacia.
 *	 >0: el mayor valor de la lista
 */
int listac::maximo_saltando(int saltar)
{
	int max, current;
	nodoc *tmp;

	max = 0;
	tmp = inicio;
	current = 0;
	// moverme al nodo numero 'saltar'
	// si saltar == 1, entonces moverme hasta el segundo nodo, osea el nodo 1.
	while( (tmp != final) && (current < saltar) ) {
		tmp = tmp->sigte;
		current++;
	};
	
	// encontrar el nodo
	while( tmp != final ) {
		if(tmp->valor > max)
			max = tmp->valor;
		tmp = tmp->sigte;
	};

	return max;	
}

/*------------------------------------------------------------------------------
 *	Retorna la catidad de nodos usados. 
 *	Osea la cantidad de nodos que tiene la lista.
 */
int listac::usados(void)
{
	int cont;
	nodoc *tmp;

	cont = 0;
	tmp = inicio;
	while( tmp != final ) {
		cont++;
		tmp = tmp->sigte;
	};

	return cont;	
} 

/*------------------------------------------------------------------------------
 *	Retorna la suma de todos los nodoc::valor de los nodos.
 */
int listac::sumatoria(void)
{
	int sum;
	nodoc *tmp;

	sum = 0;
	tmp = inicio;
	while( tmp != final ) {
		sum = sum + tmp->valor;
		tmp = tmp->sigte;
	};

	return sum;	
} 

/*------------------------------------------------------------------------------
 *	Retorna la cantidad de elementos que puede guardar, (longitud-1)
 */
int listac::capacidad(void)
{
	return (longitud-1);
}

/*------------------------------------------------------------------------------
 */
nodoc *listac::getinicio(void)
{
	return inicio;
}

/*------------------------------------------------------------------------------
 */
nodoc *listac::getfinal(void)
{
	return final;
}

/*------------------------------------------------------------------------------
 */
int listac::getlongitud(void)
{
	return longitud;
}

/*------------------------------------------------------------------------------
 */
void listac::imprimir(void)
{
	int i;
	nodoc *tmp;

	if(esta_vacia()) {
		printf("Lista vacia\n");
		return;
	}

	printf("(");
	tmp = inicio;
	for(i=1; (tmp!=final); i++) {
		printf("%d,", tmp->valor);
		tmp = tmp->sigte;
	}
	printf(")\n");
}
