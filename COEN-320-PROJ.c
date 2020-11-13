//First attempt at project implementation
//COEN 320


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




#define ONE_THOUSAND	1000
#define ONE_MILLION		1000000
/* offset and period are in microseconds. */
#define OFFSET 1000000
#define PERIOD 5000000


sem_t valueMutex;


struct CAR_VALUES
{
   float fuelConsumption;
   float engineSpeed;
   float engineCoolantTemperature;
   float currentGear;
   float transmissionOilTemperature;
   float vehicleSpeed;
   float accelerationSpeedLongitudinal;
   float indicationofbreakswitch;

}VALUES;


sem_t updateInterupt;
sem_t printMutex;
sem_t structAccess;


sem_t sigWait;

sem_t fuelFlag;
sem_t engineCoolFlag;
sem_t engineSpeedFlag;
sem_t currentGearFlag;
sem_t oilTempFlag;
sem_t speedFlag;
sem_t accelerationFlag;
sem_t breakFlag;



float currentDataFetchedArray[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int globalSecond = 0;

struct arg_struct {
    sigset_t *sigst;
    uint64_t offset;
    int period;
    int signalID;
    int columnID;
};


struct producer_struct {
    sem_t* flag;
    int id;
};




sigset_t sigst_Fuel;
sigset_t sigst_Engine;
sigset_t sigst;



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



int start_periodic_timer(void *arguments, int flag);






int main (int argc, char *argv[]) {
	pthread_t consumerpThread, timerHandlerThread;


	VALUES.fuelConsumption = 0;
	VALUES.engineSpeed = 0;
	VALUES.engineCoolantTemperature =0;
	VALUES.currentGear = 0;
	VALUES.transmissionOilTemperature = 0;
	VALUES.vehicleSpeed = 0.0;
	VALUES.accelerationSpeedLongitudinal = 0;
	VALUES.indicationofbreakswitch = 0;



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


	sigemptyset(&sigst); // initialize a signal set

	initalizeTimers();
	initalizeProducers();

	pthread_create(&timerHandlerThread, NULL, &signalHandler, NULL);

	printf("Creating the consumer producer threads.\n");
	pthread_create(&consumerpThread,NULL, &consumerThread ,NULL);



	pthread_join(&consumerpThread,NULL);
	pthread_join(&timerHandlerThread, NULL);

	pause();

	return EXIT_SUCCESS;


}




/*VARIBLES AND TIMES NEEDED
Fuel Consumption 10 ms
Engine Speed (RPM) 500 ms
Engine Coolant Temperature 2 s
Current Gear 100 ms
Transmission Oil Temperature 5 s
Vehicle Speed 100 ms
Acceleration Speed Longitudinal 150 ms
Indication of break switch 100 ms
*/

void initalizeProducers(){

	pthread_t th1, th2, th3,th4, th5, th6, th7, th8;

	struct producer_struct *fuelConsumptionStruct = (struct producer_struct *)malloc(sizeof(struct producer_struct));
	fuelConsumptionStruct->flag = &fuelFlag;
	fuelConsumptionStruct->id = 1;
	pthread_create(&th1,NULL, &producerFunction ,(void *)fuelConsumptionStruct);


	struct producer_struct *engineCoolStruct = (struct producer_struct *)malloc(sizeof(struct producer_struct));
	engineCoolStruct->flag = &engineCoolFlag;
	engineCoolStruct->id = 2;
	pthread_create(&th2,NULL, &producerFunction,(void *)engineCoolStruct);

	struct producer_struct *engineSpeed = (struct producer_struct *)malloc(sizeof(struct producer_struct));
	engineSpeed->flag = &engineSpeedFlag;
	engineSpeed->id = 3;
	pthread_create(&th3,NULL, &producerFunction ,(void *)engineSpeed);

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

void initalizeTimers(){
		//timer for the engine


		//timer for the Fuel Consumption 10 ms
		struct arg_struct argsFUEL;
		argsFUEL.sigst = &sigst;
		argsFUEL.offset = 10000;
		argsFUEL.period = 10000;
		argsFUEL.signalID = SIGUSR1;
		argsFUEL.columnID = 1;
		start_periodic_timer(&argsFUEL,2);

		//timer for the Engine Speed (RPM) 500 ms
		struct arg_struct argsENGSPEED;
		argsENGSPEED.sigst = &sigst;
		argsENGSPEED.offset = 10000;
		argsENGSPEED.period = 500000;
		argsENGSPEED.signalID = SIGUSR2;
		argsFUEL.columnID = 13;
		start_periodic_timer(&argsENGSPEED,1);

		//timer Engine Coolant Temperature 2 s
		struct arg_struct argsENGCOOL;
		argsENGCOOL.sigst = &sigst;
		argsENGCOOL.offset = 10000;
		argsENGCOOL.period = 2000000;
		argsENGCOOL.signalID = 3;
		argsFUEL.columnID = 18;
		start_periodic_timer(&argsENGCOOL,3);

		//timer Current Gear 100 ms
		struct arg_struct argsCurrentGear;
		argsCurrentGear.sigst = &sigst;
		argsCurrentGear.offset = 10000;
		argsCurrentGear.period = 100000;
		argsCurrentGear.signalID = 4;
		argsFUEL.columnID = 34;
		start_periodic_timer(&argsCurrentGear,4);

		//timer Transmission Oil Temperature 5 s
		struct arg_struct argsOilTemp;
		argsOilTemp.sigst = &sigst;
		argsOilTemp.offset = 10000;
		argsOilTemp.period = 5000000;
		argsOilTemp.signalID = 5;
		argsFUEL.columnID = 35;
		start_periodic_timer(&argsOilTemp,5);

		//timer Vehicle Speed 100 ms
		struct arg_struct argsSPEED;
		argsSPEED.sigst = &sigst;
		argsSPEED.offset = 10000;
		argsSPEED.period = 100000;
		argsSPEED.signalID = 6;
		argsFUEL.columnID = 45;
		start_periodic_timer(&argsSPEED,6);

		//timer Acceleration Speed Longitudinal 150 ms
		struct arg_struct argsACCELERATION;
		argsACCELERATION.sigst = &sigst;
		argsACCELERATION.offset = 10000;
		argsACCELERATION.period = 150000;
		argsACCELERATION.signalID = 7;
		argsFUEL.columnID = 46;
		start_periodic_timer(&argsACCELERATION,7);

		//timer Indication of break switch 100 ms
		struct arg_struct argsBREAKSWITCH;
		argsBREAKSWITCH.sigst = &sigst;
		argsBREAKSWITCH.offset = 10000;
		argsBREAKSWITCH.period = 100000;
		argsBREAKSWITCH.signalID = 8;
		argsFUEL.columnID = 47;
		start_periodic_timer(&argsBREAKSWITCH,8);


}




void* producerFunction(void* arguments)
{
	printf("PRODUCERFUNCTION");


	struct  producer_struct *args = (struct  producer_struct*)arguments;
	sem_t *flag = args->flag;
	int id  = args->id;

	static int cycles = 0;
	static uint64_t start;
	uint64_t current;
	struct timespec tv;
	double accumulativeMilli = 0;
	int currentSecond = 0;

	if (start == 0) {
		clock_gettime(CLOCK_MONOTONIC, &tv);
		start = tv.tv_sec * ONE_THOUSAND + tv.tv_nsec / ONE_MILLION;
	}

	for (;;) {
		sem_wait(flag);


		clock_gettime(CLOCK_MONOTONIC, &tv);
		current = tv.tv_sec * ONE_THOUSAND + tv.tv_nsec / ONE_MILLION;

		if (cycles > 0) {

		accumulativeMilli = accumulativeMilli + (double)(current-start)/cycles;
		currentSecond = 1000* ((accumulativeMilli)/1000000);

		//implement fetching column
		//fetchValues(consumer_Buffer);
		if(globalSecond < currentSecond){
			globalSecond = currentSecond;
		}
		printf("\nBebug Local Second_______________ = %d\n", currentSecond);
		printf("\nBebug Global Second_______________ = %d\n", globalSecond);

		printf("current seconds(row) %i \n", currentSecond);
		printf("ID %i \n", id);
		}
		if (cycles > 0) {
			//fprintf(stderr, "Ave interval between Fuel instances: %f millisecons\n",
				//(double)(current-start)/cycles);
		}

		cycles++;
		sem_wait(&structAccess);
		sem_wait(&printMutex);


		switch(id){

			case 1:
				VALUES.fuelConsumption = currentDataFetchedArray[0];
			break;

			case 2:
				VALUES.engineSpeed = currentDataFetchedArray[1];
			break;

			case 3:
				VALUES.engineCoolantTemperature = currentDataFetchedArray[2];
			break;

			case 4:
				VALUES.currentGear = currentDataFetchedArray[3];
			break;

			case 5:
				VALUES.transmissionOilTemperature = currentDataFetchedArray[4];
			break;

			case 6:
				VALUES.vehicleSpeed = currentDataFetchedArray[5];
			break;

			case 7:
				VALUES.accelerationSpeedLongitudinal = currentDataFetchedArray[6];
			break;

			case 8:
				VALUES.indicationofbreakswitch = currentDataFetchedArray[7];
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

	//Read the CSV file
	int localSecond = globalSecond;
	//Open file and Read line 1 (Title line)
	FILE* consumer = fopen("/home/DrivingKIA.csv", "r");
	char consumer_Buffer[1024];
	int columnFileID[8] = {1, 13, 18, 34, 35, 44, 45, 46};

	//Read 1st line (title)
	fgets(consumer_Buffer, 1024, consumer);

	//Execute first Read at second 0
	fgets(consumer_Buffer, 1024, consumer);

	for (;;) {

		sem_wait(&updateInterupt);
			sem_wait(&printMutex);

			if(localSecond < globalSecond){
				//Update Local Timer
				localSecond = globalSecond;

				//Fetch line
			    fgets(consumer_Buffer, 1024, consumer);

			    //Reject potential first ","
			    char * token = strtok(consumer_Buffer, ",");

			    int index = 0;
			    int columnFileTokenIndex = 0;
			    while(columnFileTokenIndex < columnFileID[7] && token != NULL){
			    	if(columnFileTokenIndex == columnFileID[index] - 1){
			    		currentDataFetchedArray[index] = atof(token);
			    		token = strtok(NULL, ",");
			    		index++;
			    	}
			    	else{
			    		//Reject Token
			    		token = strtok(NULL, ",");
			    	}
			    	columnFileTokenIndex++;
			    }
			}

			 printf("\nFuel Consumption = %f\n", VALUES.fuelConsumption);
			 printf("Engine Speed = %f\n",VALUES.engineSpeed);
			 printf("engineCoolantTemperature = %f\n",VALUES.engineCoolantTemperature);
			 printf("Current Gear =  %f\n", VALUES.currentGear);
			 printf("TransmissionOilTemperature =  %f\n", VALUES.transmissionOilTemperature);
			 printf("Vehicle Speed = %f\n",VALUES.vehicleSpeed);
			 printf("Acceleration Speed Longitudinal = %f\n", VALUES.accelerationSpeedLongitudinal);
			 printf("Indication of break switch = %f\n", VALUES.indicationofbreakswitch);
			 printf("\n");
			 printf("\n");
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
		sem_post(& speedFlag);
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









int start_periodic_timer(void *arguments, int flag){
	printf("Entered start\n");
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


	/* set timer parameters */
	timer_spec.it_value.tv_sec = offset / ONE_MILLION;			///starting time
	timer_spec.it_value.tv_nsec = (offset % ONE_MILLION) * ONE_THOUSAND;
	timer_spec.it_interval.tv_sec = period / ONE_MILLION;
	timer_spec.it_interval.tv_nsec = (period % ONE_MILLION) * ONE_THOUSAND;		//period


	sigaddset(sigst, signal); // add SIGALRM to the signal set
	sigprocmask(SIG_BLOCK, sigst, NULL); //block the signal
	printf("initalized timer.\n");
	/* set the signal event a timer expiration */
	memset(&sigev, 0, sizeof(struct sigevent));
	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = signal;
	union sigval ewanVal;
	ewanVal.sival_int = flag;
	sigev.sigev_value = ewanVal;

	/* create timer */
	res = timer_create(CLOCK_MONOTONIC, &sigev, &timer);

	if (res < 0) {
		perror("Timer Create");
		exit(-1);
	}

	timerCount++;
	/* activate the timer */
	printf("Activate.\n");
	return timer_settime(timer, 0, &timer_spec, NULL);
}

