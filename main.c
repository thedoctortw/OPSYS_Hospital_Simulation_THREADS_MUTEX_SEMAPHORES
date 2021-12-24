#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

/* Function prototypes. */
void registerationOfficeCriticalSect(int num);
int GPCriticalSect(int num, int diagnosis);
void pharmacyCriticalSect(int num);
void bloodLabCriticalSect(int num);
void ORCriticalSect(int num);
void moveToRegisterationOffice(int num);
void moveToPharmacy(int num);
void moveToBloodLab(int num);
void moveToOR(int num);
void flowMedicine(int num);
void flowBloodLab(int num, int diagnosis);
void flowSurgery(int num, int diagnosis);
int increaseRestroomNeed(int Restroom_Meter);
int resetValueToZero();
void restroomCriticalSect(int num);
void moveToRestroom(int num);
void flowRestroom(int num);
int increaseHunger(int Hunger_Meter);
void cafeCriticalSect(int num);
void moveToCafe(int num);
void flowCafe(int num);
void *patient(void *number);
void waitForMilliseconds(int waitTime);

//The number of registration desks that are available.
#define REGISTRATION_SIZE 10
//The number of restrooms that are available.
#define RESTROOM_SIZE 10
//The number of cashiers in cafe that are available.
#define CAFE_NUMBER 10
//The number of General Practitioner that are available.
#define GP_NUMBER 10
//The number of cashiers in pharmacy that are available.
#define PHARMACY_NUMBER 10
//The number of assistants in blood lab that are available.
#define BLOOD_LAB_NUMBER 10
//The number of operating rooms, surgeons and nurses that are available.
#define OR_NUMBER 10

#define SURGEON_NUMBER 30
#define NURSE_NUMBER 30
//The maximum number of surgeons and nurses that can do a surgery. A random value is
//calculated for each operation between 1 and given values to determine the required
//number of surgeons and nurses.
int SURGEON_LIMIT = 5;
int NURSE_LIMIT = 5;
//The number of patients that will be generated over the course of this program.
int PATIENT_NUMBER = 1000;
//The account of hospital where the money acquired from patients are stored.
int HOSPITAL_WALLET = 0;

int WAIT_TIME = 100;
int REGISTRATION_TIME = 100;
int GP_TIME = 200;
int PHARMACY_TIME = 100;
int BLOOD_LAB_TIME = 200;
int SURGERY_TIME = 500;
int CAFE_TIME = 100;
int RESTROOM_TIME = 100;

int REGISTRATION_COST = 100;
int PHARMACY_COST = 200; // Calculated randomly between 1 and given value.
int BLOOD_LAB_COST = 200;
int SURGERY_OR_COST = 200;
int SURGERY_SURGEON_COST = 100;
int SURGERY_NURSE_COST = 50;
int CAFE_COST = 200; // Calculated randomly between 1 and given value.

//The global increase rate of hunger and restroom needs of patients. It will increase
//randomly between 1 and given rate below.
int HUNGER_INCREASE_RATE = 10;
int RESTROOM_INCREASE_RATE = 10;

/* Global variables. */
int registerationRoomCounter = 0;
/* Semaphore declarations. */
//rooms
sem_t registerationOffice;
sem_t restroom;
sem_t cafe;
sem_t generalPractitioner;
sem_t pharmacy;
sem_t bloodLab;
sem_t operationRoom;
//staff
sem_t surgeons;
sem_t nurses;

int main(int argc, char *argv[])
{
    time_t t;
    /* Intializes random number generator */
    srand((unsigned)time(&t));

    /* Thread initialize. */
    pthread_t ptid[PATIENT_NUMBER];
    int patientId[PATIENT_NUMBER];

    // Grant each patient an ID.
    for (int i = 0; i < PATIENT_NUMBER; i++)
    {
        patientId[i] = i;
    }

    // Initialize the semaphores.
    //Rooms
    sem_init(&registerationOffice, 0, REGISTRATION_SIZE);
    sem_init(&restroom, 0, RESTROOM_SIZE);
    sem_init(&cafe, 0, CAFE_NUMBER);
    sem_init(&generalPractitioner, 0, GP_NUMBER);
    sem_init(&pharmacy, 0, PHARMACY_NUMBER);
    sem_init(&bloodLab, 0, BLOOD_LAB_NUMBER);
    sem_init(&operationRoom, 0, OR_NUMBER);

    //staff
    sem_init(&surgeons, 0, SURGEON_NUMBER);
    sem_init(&nurses, 0, NURSE_NUMBER);

    // Create patient threads.
    for (int i = 0; i < PATIENT_NUMBER; i++)
    {
        pthread_create(&ptid[i], NULL, patient, (void *)&patientId[i]);
    }

    // Join each patient thread.
    for (int i = 0; i < PATIENT_NUMBER; i++)
    {
        pthread_join(ptid[i], NULL);
    }

    // Destroy semaphores afterwards.
    sem_destroy(&registerationOffice);
    sem_destroy(&restroom);
    sem_destroy(&cafe);
    sem_destroy(&generalPractitioner);
    sem_destroy(&pharmacy);
    sem_destroy(&bloodLab);
    sem_destroy(&operationRoom);
    sem_destroy(&bloodLab);
    sem_destroy(&operationRoom);
    sem_destroy(&surgeons);
    sem_destroy(&nurses);

    printf("TOTAL HOSPITAL WALLET: %d$\n", HOSPITAL_WALLET);
}
//critical sections
void registerationOfficeCriticalSect(int num)
{
    printf(">>> Patient %d registers and pays %d$ to hospital\n", num, REGISTRATION_COST);
    HOSPITAL_WALLET += REGISTRATION_COST;
    waitForMilliseconds(WAIT_TIME + REGISTRATION_TIME);
}

int GPCriticalSect(int num, int diagnosis)
{
    printf(">>> Patient %d is entering the GP room\n", num);
    diagnosis = rand() % 3;
    //patient requires medicine
    if (diagnosis == 0)
    {
        printf(">>> GP says: Patient %d requires medicine, patient moves on to the pharmacy\n", num);
    }
    //patient requires blood test
    else if (diagnosis == 1)
    {
        printf(">>> GP says: Patient %d requires blood test, so the patient moves on...\n", num);
    }
    //patient requires surgery
    else if (diagnosis == 2)
    {
        printf(">>> GP says: Patient %d requires surgery, so the patient moves on...\n", num);
    }
    return diagnosis;
}

void pharmacyCriticalSect(int num)
{
    int MEDICINE_COST = rand() % PHARMACY_COST + 1;
    printf(">>> Patient %d buys medicine and pays %d$ to hospital\n", num, MEDICINE_COST);
    HOSPITAL_WALLET += MEDICINE_COST;
    waitForMilliseconds(WAIT_TIME + BLOOD_LAB_TIME);
}

void bloodLabCriticalSect(int num)
{
    printf(">>> Patient %d gives blood. Paid %d$ to hospital\n", num, BLOOD_LAB_COST);
    HOSPITAL_WALLET += BLOOD_LAB_COST;
}

void ORCriticalSect(int num)
{
    //check if there is enough surgeons and nurses. then decrease their number.
    int surgeonVal, nurseVal, counter;
    do
    {
        sem_getvalue(&surgeons, &surgeonVal);
        sem_getvalue(&nurses, &nurseVal);
        if (surgeonVal >= SURGEON_LIMIT && nurseVal >= NURSE_LIMIT)
        {
            for (counter = 0; counter < SURGEON_LIMIT; counter++)
            {
                sem_wait(&surgeons);
            }
            for (counter = 0; counter < NURSE_LIMIT; counter++)
            {
                sem_wait(&nurses);
            }
        }
    } while (surgeonVal < SURGEON_LIMIT || nurseVal < NURSE_LIMIT);

    waitForMilliseconds(WAIT_TIME + SURGERY_TIME);
    int TOTAL_SURGERY_COST = SURGERY_OR_COST + (SURGEON_LIMIT * SURGERY_SURGEON_COST) + (NURSE_LIMIT * SURGERY_NURSE_COST);
    printf(">>> Patient %d gets surgery and pays %d$ to hospital.\n", num, TOTAL_SURGERY_COST);
    HOSPITAL_WALLET += TOTAL_SURGERY_COST;

    //release surgeons and nurses
    for (counter = 0; counter < SURGEON_LIMIT; counter++)
    {
        sem_post(&surgeons);
    }
    for (counter = 0; counter < NURSE_LIMIT; counter++)
    {
        sem_post(&nurses);
    }
}
//hospital sections
void moveToRegisterationOffice(int num)
{
    printf(">>> Patient %d has arrived at the hospital.\n", num);
    sem_wait(&registerationOffice);
    registerationOfficeCriticalSect(num);
    sem_post(&registerationOffice);
    printf(">>> Patient %d is headed to GP\n", num);
}
void moveToPharmacy(int num)
{
    sem_wait(&pharmacy);
    pharmacyCriticalSect(num);
    printf(">>> Patient %d leaves the hospital\n", num);
    sem_post(&pharmacy);
}
void moveToBloodLab(int num)
{
    sem_wait(&bloodLab);
    bloodLabCriticalSect(num);
    printf(">>> Patient %d goes back to the GP\n", num);
    sem_post(&bloodLab);
}
void moveToOR(int num)
{
    sem_wait(&operationRoom);
    ORCriticalSect(num);
    sem_post(&operationRoom);
}

//diagnosis scenarios' workflow wrappers
void flowMedicine(int num)
{
    moveToPharmacy(num);
}
void flowBloodLab(int num, int diagnosis)
{
    moveToBloodLab(num);

    sem_wait(&generalPractitioner);
    diagnosis = rand() % 3;
    //patient is healthy
    if (diagnosis == 0)
    {
        printf(">>> GP says: Patient %d is healthy, go home.\n", num);
    }
    //patient requires blood test
    else if (diagnosis == 1)
    {
        printf(">>> GP says: Patient %d requires medicine, so the patient moves on...\n", num);
    }
    //patient requires surgery
    else if (diagnosis == 2)
    {
        printf(">>> GP says: Patient %d requires surgery, so the patient moves on...\n", num);
    }
    waitForMilliseconds(WAIT_TIME + GP_TIME);
    sem_post(&generalPractitioner);
    //patient needs medicine
    if (diagnosis == 1)
    {
        flowMedicine(num);
    }
    //patient needs surgery
    else
    {
        flowSurgery(num, diagnosis);
    }
}
void flowSurgery(int num, int diagnosis)
{
    moveToOR(num);
    //patient goes to gp post-surgery
    sem_wait(&generalPractitioner);
    diagnosis = rand() % 2;
    //patient doesn't need medication
    if (diagnosis == 0)
    {
        printf(">>> GP says: Patient %d is healthy, go home.\n", num);
        printf(">>> Patient %d leaves the hospital\n", num);
    }
    //patient requires blood test
    else if (diagnosis == 1)
    {
        printf(">>> GP says: Patient %d requires medicine, so the patient moves on...\n", num);
    }
    waitForMilliseconds(WAIT_TIME + GP_TIME);
    sem_post(&generalPractitioner);

    if (diagnosis == 1)
    {
        //patient goes to pharmacy if required
        flowMedicine(num);
    }
}

int increaseRestroomNeed(int Restroom_Meter)
{
    Restroom_Meter += rand() % RESTROOM_INCREASE_RATE + 1;
    return Restroom_Meter;
}
//to reset Restroom_Meter and Hunger_Meter
int resetValueToZero()
{
    return 0;
}

void restroomCriticalSect(int num)
{
    printf(">>> Patient %d goes to restroom\n", num);
    waitForMilliseconds(WAIT_TIME + RESTROOM_TIME);
}
void moveToRestroom(int num)
{
    sem_wait(&restroom);
    printf("Patient %d needs to go to restroom\n", num);
    restroomCriticalSect(num);
    sem_post(&restroom);
}
void flowRestroom(int num)
{
    moveToRestroom(num);
}

int increaseHunger(int Hunger_Meter)
{
    //printf("CURRENT HUNGER %d\n", Hunger_Meter);
    Hunger_Meter += rand() % HUNGER_INCREASE_RATE + 1;
    //printf("AFTER HUNGER %d\n", Hunger_Meter);
    return Hunger_Meter;
}

void cafeCriticalSect(int num)
{
    int foodPrice = rand() % CAFE_COST + 1;
    printf(">>> Patient %d goes to cafe and buys some food and drink and pays %d$\n", num, foodPrice);
    HOSPITAL_WALLET += foodPrice;
    waitForMilliseconds(WAIT_TIME + CAFE_TIME);
}

void moveToCafe(int num)
{
    sem_wait(&cafe);
    printf(">>> Patient %d is hungry\n", num);
    cafeCriticalSect(num);
    sem_post(&cafe);
}

void flowCafe(int num)
{
    moveToCafe(num);
}

void *patient(void *number)
{
    //The following patient properties needs to be created:
    int Hunger_Meter = rand() % 100 + 1; // Initialized between 1 and 100 at creation.
    int Restroom_Meter = rand() % 100 + 1; // Initialized between 1 and 100 at creation.

    //registeration office
    int num = *(int *)number;
    moveToRegisterationOffice(num);

    //RESTROOM AND HUNGER NEEDS
    Restroom_Meter = increaseRestroomNeed(Restroom_Meter);
    if (Restroom_Meter >= 100)
    {
        flowRestroom(num);
        Restroom_Meter = resetValueToZero();
    }
    Hunger_Meter = increaseHunger(Hunger_Meter);
    if (Hunger_Meter >= 100)
    {
        flowCafe(num);
        Hunger_Meter = resetValueToZero();
    }
    //General Practitioner

    sem_wait(&generalPractitioner);
    int diagnosis = GPCriticalSect(num, diagnosis);
    waitForMilliseconds(WAIT_TIME + GP_TIME);
    sem_post(&generalPractitioner);

    //RESTROOM AND HUNGER NEEDS
    Restroom_Meter = increaseRestroomNeed(Restroom_Meter);
    if (Restroom_Meter >= 100)
    {
        flowRestroom(num);
        Restroom_Meter = resetValueToZero();
    }
    Hunger_Meter = increaseHunger(Hunger_Meter);
    if (Hunger_Meter >= 100)
    {
        flowCafe(num);
        Hunger_Meter = resetValueToZero();
    }

    //patient requires medicine
    if (diagnosis == 0)
    {
        //RESTROOM AND HUNGER NEEDS
        Restroom_Meter = increaseRestroomNeed(Restroom_Meter);
        if (Restroom_Meter >= 100)
        {
            flowRestroom(num);
            Restroom_Meter = resetValueToZero();
        }
        Hunger_Meter = increaseHunger(Hunger_Meter);
        if (Hunger_Meter >= 100)
        {
            flowCafe(num);
            Hunger_Meter = resetValueToZero();
        }

        flowMedicine(num);
    }
    //patient requires blood test
    else if (diagnosis == 1)
    {
        //RESTROOM AND HUNGER NEEDS
        Restroom_Meter = increaseRestroomNeed(Restroom_Meter);
        if (Restroom_Meter >= 100)
        {
            flowRestroom(num);
            Restroom_Meter = resetValueToZero();
        }
        Hunger_Meter = increaseHunger(Hunger_Meter);
        if (Hunger_Meter >= 100)
        {
            flowCafe(num);
            Hunger_Meter = resetValueToZero();
        }

        flowBloodLab(num, diagnosis);
    }
    //patient requires surgery
    else if (diagnosis == 2)
    {
        //RESTROOM AND HUNGER NEEDS
        Restroom_Meter = increaseRestroomNeed(Restroom_Meter);
        if (Restroom_Meter >= 100)
        {
            flowRestroom(num);
            Restroom_Meter = resetValueToZero();
        }
        Hunger_Meter = increaseHunger(Hunger_Meter);
        if (Hunger_Meter >= 100)
        {
            flowCafe(num);
            Hunger_Meter = resetValueToZero();
        }

        flowSurgery(num, diagnosis);
    }
}

void waitForMilliseconds(int waitTime)
{
    usleep((rand() % waitTime + 1) * 1000);
}