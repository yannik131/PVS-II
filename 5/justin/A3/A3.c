/*
    a)
    Problem: Die Bedingung der while-Schleife (var_updated_flag == false) wird nur ein Mal geprüft,
    danach befindet sich der Thread in einer Endlosschleife (.L6: jmp .L6).
    Lösung: Verwende volatile bool var_updated_flag
    volatile bedeutet, dass der Compiler Code im Zusammenhang mit dieser Variablen nicht durch Annahmen optimieren darf.
    Im Original-Beispiel erkennt der Compiler, dass sich der Wert von var_updated_flag in thread b nicht ändern kann
    -> die Bedingung der while-Schleife ist immer falsch, wenn sie das erste Mal falsch war -> sie muss nicht mehr geprüft werden.

    Deshalb hilft auch das einkommentieren von puts: der Comiler muss davon ausgehen, dass puts den Nebeneffekt hat das var_updated_flag sich ändert
    -> der Zustand der Bedingung kann sich ändern -> die Bedingung muss wiederholt geprüft werden.

    b)
    Problem: Da es in thread a keinen Unterschied macht, ob zuerst var oder var_updated_flag gesetzt wird ist die Reihenfolge beliebig.
    Wenn var_updated_flag zuerts gesetzt wird, liest thread b noch den alten Wert von var.
    Lösung: Verwende ein memory-barrier z.B. die gcc builtin __sync_synchronize().
    Memory Zugriffe können nicht über diese Barriere hinweg getauscht werden.

    c)
    Die Barrieren, etc. werden (implizit) von den Pthread-Funktionen verwendet / verwaltet.

    d)
    volatile verhindert das Umordnen der der Instruktionen auf die volatile Variable.
    Barrieren werden implizit von Java erzeugt, wenn diese benötigt werden.

    Beim Verwenden von sychronized werden beim Betreten des gesperrten Bereichs alle geteilten Daten neu geladen
    und beim Verlassen alle geteilten Daten zurückgeschrieben.
    Das bedeutet, dass alle Änderungen immer vollständig sichtbar sind, wenn der nächste Thread auf die Daten zugreift.
    Da die Daten dadurch immer auf dem neuesten Stand sind, wird volatile nicht benötigt.
*/

#include <stdio.h>
#include <stdbool.h>

#define OLD 0
#define NEW 1

volatile bool var_updated_flag = false;
int var = OLD;

void thread_a() // executed by thread a
{
    var = NEW;
    puts("Thread a has updated var!");

    __sync_synchronize();

    var_updated_flag = true;
    puts("Thread a has notified thread b!");
}

int thread_b() // executed by thread b
{
    while(var_updated_flag == false)
    {
        // puts("Thread b is still waiting!");
    }

    puts("Thread b was notified!");

    if(var == NEW)
    {
        puts("Thread b has correctly read the new value of var!");
        return 0;
    }
    else // var == OLD
    {
        puts("Thread b has unexpectedly read the old value of var!");
        return -1;
    }
}