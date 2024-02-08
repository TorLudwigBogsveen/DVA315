#include <cairo.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>
#include "wrapper.h"

#define DT 10
static void do_drawing(cairo_t *);

GtkWidget *window;
GtkWidget *darea;

pthread_mutex_t planet_mutex = PTHREAD_MUTEX_INITIALIZER;
planet_type * planet_list = NULL;

void add_planet(planet_type* planet) {
	planet_type* list = planet_list;
	if (list == NULL) {
		planet_list = planet;
		return;
	}
	while (list->next != NULL) {
		list = list->next;
	}
	list->next = planet;
}

void remove_planet(planet_type* planet) {
    planet_type* prev = NULL;
	planet_type* list = planet_list;
	while (list != planet && list != NULL) {
        prev = list;
		list = list->next;
	}

    if (list == NULL) {
        return;
    }

    if (prev == NULL) {
        planet_list = planet->next;
    } else {
        prev->next = planet->next;
    }

	free(planet);
}

void calculate_planet_pos(planet_type *p1);

gboolean is_planet_in_map(planet_type* planet) {
	GtkAllocation alloc;
	gtk_widget_get_allocation(darea, &alloc);
	return planet->sx >= 0 && planet->sy >= 0 && planet->sx < alloc.width && planet->sy < alloc.height;
}

void * planet_thread (void*args)
{
	planet_type * this_planet = (planet_type *)args;
	while(this_planet->life > 0 && is_planet_in_map(this_planet)) {
		pthread_mutex_lock(&planet_mutex);
		calculate_planet_pos(this_planet);
		pthread_mutex_unlock(&planet_mutex);
		usleep((1.0/200.0) * 1000000);
	}

	char message[64];
	if (this_planet->life <= 0) {
		snprintf(message, sizeof(message), "%s has died", this_planet->name);
	} else {
		snprintf(message, sizeof(message), "%s has trespassed the boundaries of the map", this_planet->name);
	}


	mqd_t mq;
	MQconnect(&mq, this_planet->pid);
	MQwrite(mq, message, sizeof(message));

	pthread_mutex_lock(&planet_mutex);
	remove_planet(this_planet);
	pthread_mutex_unlock(&planet_mutex);

	pthread_exit(NULL);
}

void spawn_planet_thread(planet_type* planet) {
	pthread_t thread;
	pthread_create(&thread, NULL, planet_thread, planet);
}

void hex_to_rgba(const int color_code, double* color) {
	color[0] = (color_code >> 24 & 0xff) / 255.0;
	color[1] = (color_code >> 16 & 0xff) / 255.0;
	color[2] = (color_code >> 8 & 0xff) / 255.0;
	color[3] = (color_code >> 0 & 0xff) / 255.0;
}

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, //Draw event for cairo, will be triggered each time a draw event is executed
                              gpointer user_data)
{
    do_drawing(cr); //Launch the actual draw method
    return FALSE; //Return something
}

static void do_drawing(cairo_t *cr) //Do the drawing against the cairo surface area cr
{
    cairo_set_source_rgb(cr, 0, 0, 0); //Set RGB source of cairo, 0,0,0 = black
    cairo_select_font_face(cr, "Purisa",
          CAIRO_FONT_SLANT_NORMAL,
          CAIRO_FONT_WEIGHT_BOLD);

    //Printing planets should reasonably be done something like this:
    // --------- for all planets in list:
    // --------- cairo_arc(cr, planet.xpos, planet.ypos, 10, 0, 2*3.1415)
    // --------- cairo_fill(cr)
    //------------------------------------------Insert planet drawings below-------------------------------------------

    pthread_mutex_lock(&planet_mutex);
    planet_type *current = planet_list;
    while (current) {
    	double color[4];
    	hex_to_rgba(current->color, color);
    	cairo_set_source_rgba(cr, color[0], color[1], color[2], color[3]);
    	cairo_arc(cr, current->sx, current->sy, current->radius, 0,  2*3.1415);
    	cairo_fill(cr);
    	current = current->next;
    }

    current = planet_list;
	while (current) {
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_move_to(cr, current->sx - current->radius, current->sy - current->radius);
		cairo_show_text(cr, current->name);
		current = current->next;
	}

    pthread_mutex_unlock(&planet_mutex);
    usleep((1.0/30.0) * 1000000);

    //------------------------------------------Insert planet drawings Above-------------------------------------------

}
GtkTickCallback on_frame_tick(GtkWidget * widget, GdkFrameClock * frame_clock, gpointer user_data) //Tick handler to update the frame
{
    gdk_frame_clock_begin_updating (frame_clock); //Update the frame clock
    gtk_widget_queue_draw(darea); //Queue a draw event
    gdk_frame_clock_end_updating (frame_clock); //Stop updating frame clock
}

void calculate_planet_pos(planet_type *p1)  //Function for calculating the position of a planet, relative to all other planets in the system
{
    planet_type *current = planet_list; //Poiinter to head in the linked list
    //Variable declarations
    double Atotx = 0;
    double Atoty = 0;
    double x = 0;
    double y = 0;
    double r = 0;
    double a = 0;
    double ax = 0;
    double ay = 0;

    double G = 6.67259 * pow(10, -11); //Declaration of the gravitational constant
    while (current != NULL) //Loop through all planets in the list
    {
        if (p1 != current) //Only update variables according to properties of other planets
        {
            x = current->sx - p1->sx;
            y = current->sy - p1->sy;
            r = sqrt(pow(x, 2.0) + pow(y, 2.0));
            a = G * (current->mass / pow(r, 2.0));

            ay = a * (y / r);
            ax = a * (x / r);

            Atotx += ax;
            Atoty += ay;

        }
        current = current->next;
    }
    p1->vx = p1->vx + (Atotx * DT); //Update planet velocity, acceleration and life
    p1->vy = p1->vy + (Atoty * DT);
    p1->sx = p1->sx + (p1->vx * DT);
    p1->sy = p1->sy + (p1->vy * DT);
    p1->life -= 1;
}

void* mq_listener(void* args) {
	mqd_t* mq = args;
	while (1) {
		char buffer[1024];
		planet_type* planet = malloc(sizeof(planet_type));
		MQread(*mq, buffer, 100000);
		memcpy(planet, buffer, sizeof(planet_type));
		pthread_mutex_lock(&planet_mutex);
		add_planet(planet);
		pthread_mutex_unlock(&planet_mutex);
		spawn_planet_thread(planet);
		printf("Added planet with name: %s\n", planet->name);
		sleep(2);
	}

	pthread_exit(NULL);
}

int main(int argc, char *argv[]) //Main function
{
    //----------------------------------------Variable declarations should be placed below---------------------------------
	pthread_t mq_listener_thread;
	mqd_t mq;
    //----------------------------------------Variable declarations should be placed Above---------------------------------

    //GUI stuff, don't touch unless you know what you are doing, or if you talked to me
    gtk_init(&argc, &argv); //Initialize GTK environment
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL); //Create a new window which will serve as your top layer
    darea = gtk_drawing_area_new(); //Create draw area, which will be used under top layer window
    gtk_container_add(GTK_CONTAINER(window), darea); //add draw area to top layer window
    g_signal_connect(G_OBJECT(darea), "draw",
                     G_CALLBACK(on_draw_event), NULL); //Connect callback function for the draw event of darea
    g_signal_connect(window, "destroy", //Destroy event, not implemented yet, altough not needed
                     G_CALLBACK(gtk_main_quit), NULL);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); //Set position of window
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600); //Set size of window
    gtk_window_set_title(GTK_WINDOW(window), "GTK window"); //Title
    gtk_widget_show_all(window); //Show window
    gtk_widget_add_tick_callback(darea, on_frame_tick, NULL, 1); //Add timer callback functionality for darea
    //GUI stuff, don't touch unless you know what you are doing, or if you talked to me


    //-------------------------------Insert code for pthreads below------------------------------------------------
    //Create MQ_listener thread
    if(!MQcreate(&mq, "/planet_queue")) {
    	return EXIT_FAILURE;
    }

    pthread_create(&mq_listener_thread, NULL, mq_listener, &mq);

    //-------------------------------Insert code for pthreads above------------------------------------------------

    gtk_main();//Call gtk_main which handles basic GUI functionality

    pthread_join(mq_listener_thread, NULL);

    return EXIT_SUCCESS;
}

