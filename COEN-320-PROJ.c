///COEN 320 Project
//Ewan McNeil 40021787
//Frank Sorschak 40003535
//Kizito Kabanza40031567

#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <semaphore.h>



//time is defined in microseconds
#define ONE_THOUSAND	1000
#define ONE_MILLION		1000000
#define OFFSET 1000000
#define PERIOD 5000000




//array that is used to store the CSV file
float currentDataFetchedArray[600][8];
int globalSecond = 0;



//Shared structure acts as mailbox between
//producers and consumers
struct CAR_VALUES
{
   double fuelConsumption;
   double engineSpeed;
   double engineCoolantTemperature;
   double currentGear;
   double transmissionOilTemperature;
   double vehicleSpeed;
   double accelerationSpeedLongitudinal;
   double indicationofbreakswitch;

}VALUES;



//structures to pass to threaded functions
struct arg_struct {
    sigset_t *sigst;
    uint64_t offset;
    int period;
    int signalID;
};


struct producer_struct {
    sem_t* flag;
    int id;
};


//Semaphores needed for synchronization
sem_t updateInterupt;
sem_t printMutex;
sem_t structAccess;
sem_t valueMutex;
sem_t sigWait;
sem_t fuelFlag;
sem_t engineCoolFlag;
sem_t engineSpeedFlag;
sem_t currentGearFlag;
sem_t oilTempFlag;
sem_t speedFlag;
sem_t accelerationFlag;
sem_t breakFlag;



//signal set for each of the producers signals
sigset_t sigst;


//file stream to read in the CSV on start
FILE* stream;




//Function declarations
void *fuelConsumption(void *);
void *engineSpeed(void *);
void *engineCoolantTemperature(void *);
void *currentGear(void *);
void *transmissionOilTemperature(void *);
void *vehicleSpeed(void *);
void *accelerationSpeedLongitudinal(void *);
void *indicationofbreakswitch(void *);
void *consumerThread(void *);
void *signalHandler(void *);
void *producerFunction(void *arguments);
void initalizeTimers();
void initalizeProducers();
void fetchValues();
int start_periodic_timer(void *arguments, int flag);




int main (int argc, char *argv[]) {
	pthread_t consumerpThread, timerHandlerThread;

	printf("main started");

	//fetching values from csv
	fetchValues();


	//initalizating the values in the shared struct
	VALUES.fuelConsumption = 0.0;
	VALUES.engineSpeed = 0.0;
	VALUES.engineCoolantTemperature =0.0;
	VALUES.currentGear = 0.0;
	VALUES.transmissionOilTemperature = 0.0;
	VALUES.vehicleSpeed = 0.0;
	VALUES.accelerationSpeedLongitudinal = 0.0;
	VALUES.indicationofbreakswitch = 0.0;

	//initalizting semaphores
	sem_init(&structAccess, 0, 1);
	sem_init(&valueMutex, 0, 1);
	sem_init(&updateInterupt, 0, 0);
	sem_init(&printMutex,0,1);
	sem_init(&sigWait,0,1);
	sem_init(&fuelFlag,0,1);
	sem_init(&engineCoolFlag,0,1);
	sem_init(& engineSpeedFlag,0,1);
	sem_init(&currentGearFlag,0,1);
	sem_init(&oilTempFlag,0,1);
	sem_init(& speedFlag,0,1);
	sem_init(&accelerationFlag,0,1);
	sem_init(&breakFlag,0,1);

	//initialize a signal set
	sigemptyset(&sigst);


	//functions that call the initalizations of the
	//signals in the signal set
	//and the producer threads
	initalizeTimers();
	initalizeProducers();




	//threads for the timerhandler and the consumer
	pthread_create(&timerHandlerThread, NULL, &signalHandler, NULL);
	pthread_create(&consumerpThread,NULL, &consumerThread ,NULL);



	pthread_join(&consumerpThread,NULL);
	pthread_join(&timerHandlerThread, NULL);

	 pause();




		return EXIT_SUCCESS;


}



//Here the Eight producer threads are initialized with the
//proper periods set
void initalizeProducers(){

	pthread_t th1, th2, th3,th4, th5, th6, th7, th8;

	struct producer_struct *fuelConsumptionStruct = (struct producer_struct *)malloc(sizeof(struct producer_struct));
	fuelConsumptionStruct->flag = &fuelFlag;
	fuelConsumptionStruct->id = 1;
	pthread_create(&th1,NULL, &producerFunction ,(void *)fuelConsumptionStruct);


	struct producer_struct *engineSpeed = (struct producer_struct *)malloc(sizeof(struct producer_struct));
	engineSpeed->flag = &engineSpeedFlag;
	engineSpeed->id = 2;
	pthread_create(&th3,NULL, &producerFunction ,(void *)engineSpeed);


	struct producer_struct *engineCoolStruct = (struct producer_struct *)malloc(sizeof(struct producer_struct));
	engineCoolStruct->flag = &engineCoolFlag;
	engineCoolStruct->id = 3;
	pthread_create(&th2,NULL, &producerFunction,(void *)engineCoolStruct);

	struct producer_struct *currentGear = (struct producer_struct *)malloc(sizeof(struct producer_struct));
	currentGear->flag = &currentGearFlag;
	currentGear->id = 4;
	pthread_create(&th4,NULL, &producerFunction ,(void *)currentGear);

	struct producer_struct *oilTemp = (struct producer_struct *)malloc(sizeof(struct producer_struct));
	oilTemp->flag = &oilTempFlag;
	oilTemp->id = 5;
	pthread_create(&th5,NULL, &producerFunction ,(void *)oilTemp);

	struct producer_struct *speed = (struct producer_struct *)malloc(sizeof(struct producer_struct));
	speed->flag = &speedFlag;
	speed->id = 6;
	pthread_create(&th6,NULL, &producerFunction ,(void *)speed);

	struct producer_struct *acceleration = (struct producer_struct *)malloc(sizeof(struct producer_struct));
	acceleration->flag = &accelerationFlag;
	acceleration->id = 7;
	pthread_create(&th7,NULL, &producerFunction ,(void *)acceleration);

	struct producer_struct *breakStruct = (struct producer_struct *)malloc(sizeof(struct producer_struct));
	breakStruct->flag = &breakFlag;
	breakStruct->id = 8;
	pthread_create(&th8,NULL, &producerFunction ,(void *)breakStruct);

	pthread_join(&th1,NULL);
	pthread_join(&th2,NULL);
	pthread_join(&th3,NULL);
	pthread_join(&th4,NULL);
	pthread_join(&th5,NULL);
	pthread_join(&th6,NULL);
	pthread_join(&th7,NULL);
	pthread_join(&th8,NULL);
}


//here the periodic signals for the producers are created
//(may be poorly named they are periodi signals not nessicariy timers being created)
void initalizeTimers(){
		//timer for the engine


		//timer for the Fuel Consumption 10 ms
		struct arg_struct argsFUEL;
		argsFUEL.sigst = &sigst;
		argsFUEL.offset = 10000;
		argsFUEL.period = 10000;
		argsFUEL.signalID = SIGUSR1;
		start_periodic_timer(&argsFUEL,1);

		//timer for the Engine Speed (RPM) 500 ms
		struct arg_struct argsENGSPEED;
		argsENGSPEED.sigst = &sigst;
		argsENGSPEED.offset = 10000;
		argsENGSPEED.period = 500000;
		argsENGSPEED.signalID = SIGUSR2;
		start_periodic_timer(&argsENGSPEED,2);

		//timer Engine Coolant Temperature 2 s
		struct arg_struct argsENGCOOL;
		argsENGCOOL.sigst = &sigst;
		argsENGCOOL.offset = 10000;
		argsENGCOOL.period = 2000000;
		argsENGCOOL.signalID = 3;
		start_periodic_timer(&argsENGCOOL,3);

		//timer Current Gear 100 ms
		struct arg_struct argsCurrentGear;
		argsCurrentGear.sigst = &sigst;
		argsCurrentGear.offset = 10000;
		argsCurrentGear.period = 100000;
		argsCurrentGear.signalID = 4;
		start_periodic_timer(&argsCurrentGear,4);

		//timer Transmission Oil Temperature 5 s
		struct arg_struct argsOilTemp;
		argsOilTemp.sigst = &sigst;
		argsOilTemp.offset = 10000;
		argsOilTemp.period = 5000000;
		argsOilTemp.signalID = 5;
		start_periodic_timer(&argsOilTemp,5);

		//timer Vehicle Speed 100 ms
		struct arg_struct argsSPEED;
		argsSPEED.sigst = &sigst;
		argsSPEED.offset = 10000;
		argsSPEED.period = 100000;
		argsSPEED.signalID = 6;
		start_periodic_timer(&argsSPEED,6);

		//timer Acceleration Speed Longitudinal 150 ms
		struct arg_struct argsACCELERATION;
		argsACCELERATION.sigst = &sigst;
		argsACCELERATION.offset = 10000;
		argsACCELERATION.period = 150000;
		argsACCELERATION.signalID = 7;
		start_periodic_timer(&argsACCELERATION,7);

		//timer Indication of break switch 100 ms
		struct arg_struct argsBREAKSWITCH;
		argsBREAKSWITCH.sigst = &sigst;
		argsBREAKSWITCH.offset = 10000;
		argsBREAKSWITCH.period = 100000;
		argsBREAKSWITCH.signalID = 8;
		start_periodic_timer(&argsBREAKSWITCH,8);


}




void* producerFunction(void* arguments)
{

	fprintf(stderr, "PRODUCERFUNCTION");


	struct  producer_struct *args = (struct  producer_struct*)arguments;
	sem_t *flag = args->flag;
	int id  = args->id;

	int cycles = 0;
	uint64_t start;
	uint64_t current;
	struct timespec tv;
	double accumulativeMilli = 0;
	int currentSecond = 0;

	if (start == 0) {
		clock_gettime(CLOCK_MONOTONIC, &tv);
		start = tv.tv_sec * ONE_THOUSAND + tv.tv_nsec / ONE_MILLION;
	}

	for (;;) {
		//wait for the signal handler to tell it when to produce
		sem_wait(flag);


		clock_gettime(CLOCK_MONOTONIC, &tv);
		current = tv.tv_sec * ONE_THOUSAND + tv.tv_nsec / ONE_MILLION;

		//Time logic in order to get the average value of the period
		//this has been used from the logic seen in the tutorial
		if (cycles > 0) {
		accumulativeMilli = accumulativeMilli + (double)(current-start)/cycles;
		currentSecond = 1000* ((accumulativeMilli)/1000000);


		if(globalSecond < currentSecond){
					globalSecond = currentSecond;
				}
		else{
			currentSecond = globalSecond;
		}

		}
		if (cycles > 0) {


			printf("Producer: %i has the average interval of: %f milliseconds\n ", id,(double)(current-start)/cycles);

		}

		cycles++;
		sem_wait(&structAccess);
		sem_wait(&printMutex);


		//based on the ID the producer gets the value from the input array and loads it into the shared
		//Structure which serves as the mailbox for the producer and consumers.
		switch(id){

		case 1:
						VALUES.fuelConsumption = currentDataFetchedArray[globalSecond][0];
					break;

					case 2:
						VALUES.engineSpeed = currentDataFetchedArray[globalSecond][1];
					break;

					case 3:
						VALUES.engineCoolantTemperature = currentDataFetchedArray[globalSecond][2];
					break;

					case 4:
						VALUES.currentGear = currentDataFetchedArray[globalSecond][3];
					break;

					case 5:
						VALUES.transmissionOilTemperature = currentDataFetchedArray[globalSecond][4];
					break;

					case 6:
						VALUES.vehicleSpeed = currentDataFetchedArray[globalSecond][5];
					break;

					case 7:
						VALUES.accelerationSpeedLongitudinal = currentDataFetchedArray[globalSecond][6];
					break;

					case 8:
						VALUES.indicationofbreakswitch = currentDataFetchedArray[globalSecond][7];
					break;
			}


		sem_post(&printMutex);
		sem_post(&structAccess);
		sem_post(&updateInterupt);


	}
	return NULL;
}








void *consumerThread(void *empty)
{

	//file init



	for (;;) {

		//wait call for the signal from a producer that has just produced
		sem_wait(&updateInterupt);
			sem_wait(&printMutex);

			//make a lock file
			FILE* lock = fopen("/home/lock.txt", "w");
			fclose(lock);

			FILE* out = fopen("/home/currentData.csv", "w");


			 //outputing all the values from the producer consumer "mailbox"
			 printf("CURRENT TIME: %i \n ", globalSecond);
			 printf("Fuel Consumption = %lf\n", VALUES.fuelConsumption);
			 printf("Engine Speed = %lf\n",VALUES.engineSpeed);
			 printf("engineCoolantTemperature = %lf\n",VALUES.engineCoolantTemperature);
			 printf("Current Gear =  %lf\n", VALUES.currentGear);
			 printf("transmissionOilTemperature =  %lf\n", VALUES.transmissionOilTemperature);
			 printf("Vehicle Speed = %lf\n",VALUES.vehicleSpeed);
			 printf("Acceleration Speed Longitudinal = %lf\n", VALUES.accelerationSpeedLongitudinal);
			 printf("Indication of break switch = %lf\n", VALUES.indicationofbreakswitch);
			 printf("\n");
			 printf("\n");

			 fprintf(out, "%d", globalSecond);

			 fputs(",", out);
			 fprintf(out, "%f", VALUES.fuelConsumption);
			 fputs(",", out);
			 fprintf(out, "%f", VALUES.engineSpeed);
			 fputs(",", out);
			 fprintf(out, "%f", VALUES.engineCoolantTemperature);
			 fputs(",", out);
			 fprintf(out, "%f", VALUES.currentGear);
			 fputs(",", out);
			 fprintf(out, "%f", VALUES.transmissionOilTemperature);
			 fputs(",", out);
			 fprintf(out, "%f", VALUES.vehicleSpeed);
			 fputs(",", out);
			 fprintf(out, "%f", VALUES.accelerationSpeedLongitudinal);
			 fputs(",", out);
			 fprintf(out, "%f", VALUES.indicationofbreakswitch);


			 fclose(out);
			 remove("/home/lock.txt");




		sem_post(&printMutex);


	}

	return(NULL);
}






void *signalHandler(void *empty){
	siginfo_t info;
	union sigval output;
	for (;;) {
	sigwaitinfo(&sigst, &info);
	output = info.__data.__proc.__pdata.__kill.__value;

	switch(output.sival_int){

	case 1:
		sem_post(&fuelFlag);
	break;

	case 2:
		sem_post(&engineSpeedFlag);
	break;

	case 3:
		sem_post(&engineCoolFlag);
	break;

	case 4:
		sem_post(&currentGearFlag);
	break;

	case 5:
		sem_post(&oilTempFlag);
	break;

	case 6:
		sem_post(&speedFlag);
	break;

	case 7:
		sem_post(&accelerationFlag);
	break;
	case 8:
		sem_post(&breakFlag);
	break;
	}

	}
}



//This is an edited version of the start_periodic_timer seen in the tutorial
//The changes come in with extra parameters being passed in.
int start_periodic_timer(void *arguments, int flag){
	//taking in the arguments passed through a structure that was added into the function
	struct arg_struct *args = (struct arg_struct *)arguments;
	sigset_t *sigst = args -> sigst;
	uint64_t offset = args -> offset;
	int period = args -> period;
	int signalID = args -> signalID;


	struct itimerspec timer_spec;
	struct sigevent sigev;
	timer_t timer;
	static int timerCount = 1;
	int signal = signalID;
	int res;
	printf("ARG PERIOD.\n");

	//offsets and periods from the input structure are passed in
	timer_spec.it_value.tv_sec = offset / ONE_MILLION;
	timer_spec.it_value.tv_nsec = (offset % ONE_MILLION) * ONE_THOUSAND;
	timer_spec.it_interval.tv_sec = period / ONE_MILLION;
	timer_spec.it_interval.tv_nsec = (period % ONE_MILLION) * ONE_THOUSAND;




	sigaddset(sigst, signal);
	sigprocmask(SIG_BLOCK, sigst, NULL);
	printf("Initialized timer.\n");
	memset(&sigev, 0, sizeof(struct sigevent));
	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = signal;



	//This additional Identifier is crucial for the program in order to
	//pass the ID flag that is later used in the signalHandler
	//to identify the signal via the sigwaitinfo() call
	union sigval addionalIdentifier;
	addionalIdentifie.sival_int = flag;
	sigev.sigev_value = addionalIdentifie;


	//Timer creation and activation
	res = timer_create(CLOCK_MONOTONIC, &sigev, &timer);
	if (res < 0) {
		perror("Timer Create");
		exit(-1);
	}
	timerCount++;
	printf("Activate.\n");
	return timer_settime(timer, 0, &timer_spec, NULL);
}




void fetchValues(){
	//Read the CSV file
	//Open file and Read line 1 (Title line)
	FILE* consumer = fopen("/home/DrivingKIA.csv", "r");
	char consumer_Buffer[1024];
	int columnFileID[8] = {1, 13, 18, 34, 35, 44, 45, 46};

	//Read 1st line (title)
	fgets(consumer_Buffer, 1024, consumer);
	fgets(consumer_Buffer, 1024, consumer);
	int dataCounter = 0;
	while (dataCounter < 600){
		//Fetch line
		fgets(consumer_Buffer, 1024, consumer);

		//Reject potential first ","
		char * token = strtok(consumer_Buffer, ",");

		int index = 0;
		int columnFileTokenIndex = 0;
		while(columnFileTokenIndex < columnFileID[7] && token != NULL){
			if(columnFileTokenIndex == columnFileID[index] - 1){
				currentDataFetchedArray[dataCounter][index] = atof(token);
				token = strtok(NULL, ",");
				index++;
			}
			else{
				//Reject Token
				token = strtok(NULL, ",");
			}
			columnFileTokenIndex++;
		}
		dataCounter++;
	}

	fclose(consumer);

	int arrayCount = 0;
	while(arrayCount < 600){

		printf("%i, %f, %f,%f,%f,%f, %f,%f,%f \n ", arrayCount,currentDataFetchedArray[arrayCount][0],currentDataFetchedArray[arrayCount][1],currentDataFetchedArray[arrayCount][2],currentDataFetchedArray[arrayCount][3],currentDataFetchedArray[arrayCount][4],currentDataFetchedArray[arrayCount][5],currentDataFetchedArray[arrayCount][6],currentDataFetchedArray[arrayCount][7]);
		arrayCount++;
	}
}







