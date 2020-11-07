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
   double fuelConsumption;
   double engineSpeed;
   double engineCoolantTemperature;
   double currentGear;
   double transmissionOilTemperature;
   double vehicleSpeed;
   double accelerationSpeedLongitudinal;
   double indicationofbreakswitch;

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





struct arg_struct {
    sigset_t *sigst;
    uint64_t offset;
    int period;
    int signalID;
};


sigset_t sigst_Fuel;
sigset_t sigst_Engine;
sigset_t sigst;
FILE* stream;


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
void initalizeTimers();


int start_periodic_timer(void *arguments, int flag);




int main (int argc, char *argv[]) {
	pthread_t th1, th2, th3,th4, th5, th6, th7, th8, consumerpThread, timerHandlerThread;


	stream = fopen("Driving Data(KIA SOUL)_(150728-160714)_(10 Drivers_A-J).csv", "r");

	VALUES.fuelConsumption = 0.0;
	VALUES.engineSpeed = 0.0;
	VALUES.engineCoolantTemperature =0.0;
	VALUES.currentGear = 0.0;
	VALUES.transmissionOilTemperature = 0.0;
	VALUES.vehicleSpeed = 0.0;
	VALUES.accelerationSpeedLongitudinal = 0.0;
	VALUES.indicationofbreakswitch = 0.0;

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


	pthread_create(&timerHandlerThread, NULL, &signalHandler, NULL);

	printf("Creating the consumer producer threads.\n");
	pthread_create(&consumerpThread,NULL, &consumerThread ,NULL);
	pthread_create(&th1,NULL, &fuelConsumption ,NULL);
	pthread_create(&th2,NULL, &engineSpeed ,NULL);
	pthread_create(&th3,NULL, &engineCoolantTemperature, NULL);
//	pthread_create(&th4,NULL, &fuelConsumption ,NULL);
//	pthread_create(&th5,NULL, &engineSpeed ,NULL);
//	pthread_create(&th6,NULL, &engineCoolantTemperature, NULL);
//	pthread_create(&th7,NULL, &fuelConsumption ,NULL);
//	pthread_create(&th8,NULL, &engineSpeed ,NULL);


	pthread_join(&consumerpThread,NULL);
	pthread_join(&timerHandlerThread, NULL);
	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	pthread_join(th3, NULL);
//	pthread_join(th4, NULL);
//	pthread_join(th5, NULL);
//	pthread_join(th6, NULL);
//	pthread_join(th7, NULL);
//	pthread_join(th8, NULL);
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


void initalizeTimers(){
		//timer for the engine


		//timer for the Fuel Consumption 10 ms
		struct arg_struct argsFUEL;
		argsFUEL.sigst = &sigst;
		argsFUEL.offset = 10000;
		argsFUEL.period = 10000;
		argsFUEL.signalID = SIGUSR1;
		start_periodic_timer(&argsFUEL,2);

		//timer for the Engine Speed (RPM) 500 ms
		struct arg_struct argsENGSPEED;
		argsENGSPEED.sigst = &sigst;
		argsENGSPEED.offset = 10000;
		argsENGSPEED.period = 500000;
		argsENGSPEED.signalID = SIGUSR2;
		start_periodic_timer(&argsENGSPEED,1);

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
		start_periodic_timer(&argsBREAKSWITCH,7);


}

void *fuelConsumption(void *empty)
{



	static int cycles = 0;
	static uint64_t start;
	uint64_t current;
	struct timespec tv;
	uint64_t accumulativeMilli = 0;
	int currentSecond = 0;

	if (start == 0) {
		clock_gettime(CLOCK_MONOTONIC, &tv);
		start = tv.tv_sec * ONE_THOUSAND + tv.tv_nsec / ONE_MILLION;
	}

	for (;;) {
		sem_wait(&fuelFlag);


		clock_gettime(CLOCK_MONOTONIC, &tv);
		current = tv.tv_sec * ONE_THOUSAND + tv.tv_nsec / ONE_MILLION;
		accumulativeMilli = accumulativeMilli + current;
		currentSecond = 1000* ((double)accumulativeMilli/1000);


		//round current to the lower second to get the line in the CSV we want
		fprintf(stderr, "current seconds(row) %f \n", currentSecond);
		if (cycles > 0) {
			//fprintf(stderr, "Ave interval between Fuel instances: %f millisecons\n",
				//(double)(current-start)/cycles);
		}

		cycles++;
		sem_wait(&structAccess);
		sem_wait(&printMutex);
		sem_post(&printMutex);
		VALUES.fuelConsumption =VALUES.fuelConsumption + 2.0;
		sem_post(&structAccess);
		sem_post(&updateInterupt);


	}
	return NULL;
}



void *engineSpeed(void *empty){

	static int cycles = 0;
	static uint64_t start;
	uint64_t current;
	struct timespec tv;

	if (start == 0) {
		clock_gettime(CLOCK_MONOTONIC, &tv);
		start = tv.tv_sec * ONE_THOUSAND + tv.tv_nsec / ONE_MILLION;
	}



		for (;;) {
			sem_wait(&engineSpeedFlag);


			clock_gettime(CLOCK_MONOTONIC, &tv);
			current = tv.tv_sec * ONE_THOUSAND + tv.tv_nsec / ONE_MILLION;

			if (cycles > 0) {
				//fprintf(stderr, "Ave interval between instances engineSpeed : %f millisecons\n",
				//	(double)(current-start)/cycles);
			}

			cycles++;
			sem_wait(&printMutex);

			sem_post(&printMutex);

			sem_wait(&structAccess);
			VALUES.engineSpeed = VALUES.engineSpeed + 8.0;
			sem_post(&structAccess);

			sem_post(&updateInterupt);



		}

		return NULL;



}




void *engineCoolantTemperature(void *empty)
{



	static int cycles = 0;
	static uint64_t start;
	uint64_t current;
	struct timespec tv;

	if (start == 0) {
		clock_gettime(CLOCK_MONOTONIC, &tv);
		start = tv.tv_sec * ONE_THOUSAND + tv.tv_nsec / ONE_MILLION;
	}

	for (;;) {
		sem_wait(&engineCoolFlag);


		clock_gettime(CLOCK_MONOTONIC, &tv);
		current = tv.tv_sec * ONE_THOUSAND + tv.tv_nsec / ONE_MILLION;

		if (cycles > 0) {
			//fprintf(stderr, "Ave interval between Engine coolant instances: %f millisecons\n",
			//	(double)(current-start)/cycles);
		}

		cycles++;
		sem_wait(&structAccess);
		sem_wait(&printMutex);
		sem_post(&printMutex);
		//VALUES.fuelConsumption =VALUES.fuelConsumption + 2.0;
		sem_post(&structAccess);
		sem_post(&updateInterupt);


	}
	return NULL;
}









//void *currentGear(void *);
//void *transmissionOilTemperature(void *);
//void *vehicleSpeed(void *);
//void *accelerationSpeedLongitudinal(void *);
//void *indicationofbreakswitch(void *);




void *consumerThread(void *empty)
{

	for (;;) {
		sem_wait(&updateInterupt);
			sem_wait(&printMutex);
//			 printf("Fuel Consumption = %lf\n", VALUES.fuelConsumption);
//			 printf("Engine Speed = %lf\n",VALUES.engineSpeed);
////			 printf("engineCoolantTemperature = %lf\n",VALUES.engineCoolantTemperature);
////			 printf("Current Gear =  %lf\n", VALUES.currentGear);
////			 printf("transmissionOilTemperature =  %lf\n", VALUES.transmissionOilTemperature);
////			 printf("Vehicle Speed = %lf\n",VALUES.vehicleSpeed);
////			 printf("Acceleration Speed Longitudinal = %lf\n", VALUES.accelerationSpeedLongitudinal);
////			 printf("Indication of break switch = %lf\n", VALUES.indicationofbreakswitch);
//			 printf("\n");
//			 printf("\n");
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


	}

	}
}



const char* getfield(char* line, int num)
{
    const char* tok;

    for (tok = strtok(line, ";");
            tok && *tok;
            tok = strtok(NULL, ";\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
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









