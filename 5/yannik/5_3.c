/**
 * a) Bei aggressiven Optimierungen stellt der Compiler fest, dass der Wert von var_updated_flag
 * innerhalb der while-Schleife nicht veraendert werden kann. Solange eine Variable nicht mit
 * volatile gekennzeichnet wird, geht der Compiler nicht davon aus, dass der Wert von außerhalb
 * geaendert werden kann und ueberprueft ihn daher nur einmal.
 * Bonuspunkt: Die Antwort ist doch in der Aufgabe gegeben?! Da puts potenziell var_updated_flag
 * veraendern koennte, muss der Wert nach jedem Aufruf wieder neu geladen und verglichen werden.
 *
 * b) Der Compiler folgt der as-if-rule: "The rule that allows any and all code transformations that
 * do not change the observable behavior of the program". In Einzelthread-Code duerfen Anweisungen
 * in unterschiedlichen Reihenfolgen ausgefuehrt werden, was die Reihenfolge der Anweisungen
 * generiert vom Compiler und zur Ausfuehrungszeit auf der CPU beeinflussen kann, solange sich
 * dadurch das Verhalten des Programms im Einzelthread-Modus nicht aendert. Beim Multithreading ist
 * die Reihenfolge von Speicherzugriffen zwischen den verschiedenen Threads meist nicht sequentiell,
 * was zu dem beobachteten Phaenomen fuehrt. Loesen laesst sich das, indem Memory-Ordering explizit
 * angegeben wird (vgl. https://en.cppreference.com/w/cpp/atomic/memory_order.html)
 *
 * c) Weil Pthread-Objekte eben genau das kapseln und intern verwenden
   d)
    volatile verhindert das Umordnen der der Instruktionen auf die volatile Variable.
    Barrieren werden implizit von Java erzeugt, wenn diese benötigt werden.

    Beim Verwenden von sychronized werden beim Betreten des gesperrten Bereichs alle geteilten Daten
   neu geladen und beim Verlassen alle geteilten Daten zurueckgeschrieben. Das bedeutet, dass alle
   Aenderungen immer vollstaendig sichtbar sind, wenn der naechste Thread auf die Daten zugreift. Da
   die Daten dadurch immer auf dem neuesten Stand sind, wird volatile nicht benoetigt.
 */

#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>

#define OLD 0
#define NEW 1
atomic_bool var_updated_flag = false;
int var = OLD;

void thread_a() // executed by thread a
{
    var = NEW;
    puts("Thread a has updated var!");

    // Alternativ kann man auch eine Memory-Barrier explizit angeben mit (gcc-builtin):
    // __sync_synchronize();

    // memory_order_release erzwingt, dass die vorige var = NEW Zuweisung in anderen Threads
    // sichtbar ist und nicht umsortiert werden kann
    atomic_store_explicit(&var_updated_flag, true, memory_order_release);
    puts("Thread a has notified thread b!");
}

int thread_b() // executed by thread b
{
    while (!atomic_load_explicit(&var_updated_flag, memory_order_acquire)) {
        // puts("Thread b is still waiting!");
    }
    puts("Thread b was notified!");
    if (var == NEW) {
        puts("Thread b has correctly read the new value of var!");
        return 0;
    } else // var == OLD
    {
        puts("Thread b has unexpectedly read the old value of var!");
        return -1;
    }
}