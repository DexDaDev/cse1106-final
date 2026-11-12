#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <pigpio.h>

#define LED_PIN 17
#define LOG_FILE "../data/occupancy_log.txt"

typedef struct
{
    int occupancy;
    int capacity;
} Room;
FILE *file;

void displayStatus(Room room);
void updateLED(Room room);
void saveEvent(char event[], Room room);
void recordEntry(Room *room);
void recordExit(Room *room);
void changeCapacity(Room *room);

int main(int argc, char* argv[])
{
    Room room = {0, 5};
    int choice;

    gpioInitialise();

    gpioSetMode(LED_PIN, PI_OUTPUT);
    updateLED(room);

    file = fopen(LOG_FILE, "w+");
    if (file == NULL)
    {
        printf("Cannot open log file!\n");
        return 0;
    }

    while (true)
    {
        printf("\n========== MENU ==========\n");
        printf("1. Record person entering\n");
        printf("2. Record person exiting\n");
        printf("3. Display room status\n");
        printf("4. Change maximum capacity\n");
        printf("5. Exit\n");
        printf("==========================\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            recordEntry(&room);
            break;

        case 2:
            recordExit(&room);
            break;

        case 3:
            displayStatus(room);
            break;

        case 4:
            changeCapacity(&room);
            break;

        case 5:
            printf("Program closing...\n\n");
            gpioWrite(LED_PIN, 0);
            gpioTerminate();
            return 0;

        default:
            printf("Invalid choice!\n\n");
        }
    }
    fclose(file);
}

void displayStatus(Room room)
{
    bool isFull = (room.occupancy >= room.capacity);

    printf("\n------ ROOM STATUS ------\n");
    printf("Current occupancy: %d\n", room.occupancy);
    printf("Maximum capacity: %d\n", room.capacity);

    if (isFull)
        printf("Room status: ROOM FULL\n");
    else
        printf("Room status: AVAILABLE\n");

    printf("-------------------------\n");
}
void updateLED(Room room)
{
    bool isFull = (room.occupancy >= room.capacity);

    if (isFull)
        gpioWrite(LED_PIN, 1);
    else
        gpioWrite(LED_PIN, 0);
}
void saveEvent(char event[], Room room)
{
    fseek(file, 0, SEEK_END);

    if (ftell(file) == 0)
    {
        fprintf(file,"Event\t\t\t\tOccupancy\t\t\t\tCapacity\n");
    }

    fprintf(file, "%-20s%-24d%d\n", event, room.occupancy, room.capacity);
}
void recordEntry(Room *room)
{
    if (room->occupancy >= room->capacity)
    {
        printf("Room is full! Entry rejected.\n");
        saveEvent("ENTRY_REJECTED", *room);
        return;
    }

    room->occupancy++;

    printf("Person entered successfully.\n");

    updateLED(*room);
    saveEvent("ENTRY", *room);
    // displayStatus(*room);
}
void recordExit(Room *room)
{
    if (room->occupancy <= 0)
    {
        printf("Room is empty! Exit rejected.\n");
        saveEvent("EXIT_REJECTED", *room);
        return;
    }

    room->occupancy--;

    printf("Person exited successfully.\n");

    updateLED(*room);
    saveEvent("EXIT", *room);
    // displayStatus(*room);
}
void changeCapacity(Room *room)
{
    int newCapacity;

    printf("Enter new maximum capacity: ");
    scanf("%d", &newCapacity);

    if (newCapacity <= 0)
    {
        printf("Capacity must be greater than zero.\n");
        return;
    }

    if (newCapacity < room->occupancy)
    {
        printf("Cannot set capacity below current occupancy!\n");
        return;
    }

    room->capacity = newCapacity;

    printf("Capacity updated successfully!\n");

    updateLED(*room);
    saveEvent("CAPACITY_CHANGED", *room);
    // displayStatus(*room);
}