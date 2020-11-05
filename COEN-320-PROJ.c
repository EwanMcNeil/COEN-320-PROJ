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
sem_t engineFlag;

struct arg_struct {
    sigset_t *sigst;
    uint64_t offset;
    int period;
    int signalID;
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


int start_periodic_timer(void *arguments, int flag);
static void wait_next_activation(sigset_t *sigst);

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


int main (int argc, char *argv[]) {
	pthread_t th1, th2, th3, timerHandler;

	VALUES.fuelConsumption = 2.0;
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
	sem_init(&engineFlag,0,1);

	sigemptyset(&sigst); // initialize a signal set


	struct arg_struct argsENG;
	argsENG.sigst = &sigst;
	argsENG.offset = 10000;
	argsENG.period = 1500000;
	argsENG.signalID = SIGUSR2;



	start_periodic_timer(&argsENG,1);


	//pthread_t timeThreadFUEL;
	struct arg_struct argsFUEL;
	argsFUEL.sigst = &sigst;
	argsFUEL.offset = 10000;
	argsFUEL.period = 3000000;
	argsFUEL.signalID = SIGUSR1;



	start_periodic_timer(&argsFUEL,2);


	pthread_create(&timerHandler, NULL, &signalHandler, NULL);

//	printf("Creating the consumer producer threads.\n");
//	pthread_create(&th1,NULL, &consumerThread ,NULL);
//	pthread_create(&th2,NULL, &fuelConsumption ,NULL);
//	pthread_create(&th3,NULL, &engineSpeed ,NULL);


	pthread_join(&timerHandler, NULL);
	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	pthread_join(th3, NULL);
	 pause();



		return EXIT_SUCCESS;





}



void *fuelConsumption(void *empty)
{

	int dummy;
	dummy  = 5;
	for (;;) {

		sem_wait(&fuelFlag);
		sem_wait(&structAccess);

		VALUES.fuelConsumption =VALUES.fuelConsumption + 2.0;
		sem_post(&structAccess);
		sem_post(&updateInterupt);

		sem_post(&fuelFlag);

	}
	return NULL;
}



void *engineSpeed(void *empty){

		int count = 0;
		for (;;) {

			sem_wait(&engineFlag);
			sem_wait(&printMutex);

			sem_post(&printMutex);

			sem_wait(&structAccess);
			count = count +1;
			VALUES.engineSpeed = 20*count*.8;
			sem_post(&structAccess);


			//wait_next_activation(); //wait for timer expiration
			sem_post(&updateInterupt);

			sem_post(&engineFlag);

		}

		return NULL;



}





void *consumerThread(void *empty)
{

	for (;;) {
		sem_wait(&updateInterupt);
			sem_wait(&printMutex);
			 printf("Fuel Consumption = %lf\n", VALUES.fuelConsumption);
			 printf("Engine Speed = %lf\n",VALUES.engineSpeed);
			 printf("engineCoolantTemperature = %lf\n",VALUES.engineCoolantTemperature);
			 printf("Current Gear =  %lf\n", VALUES.currentGear);
			 printf("transmissionOilTemperature =  %lf\n", VALUES.transmissionOilTemperature);
			 printf("Vehicle Speed = %lf\n",VALUES.vehicleSpeed);
			 printf("Acceleration Speed Longitudinal = %lf\n", VALUES.accelerationSpeedLongitudinal);
			 printf("Indication of break switch = %lf\n", VALUES.indicationofbreakswitch);
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
	printf("signo= %d\n", output.sival_int);


	switch(output.sival_int){

	case 1:
		sem_post(&fuelFlag);
	break;

	case 2:
		sem_post(&engineFlag);
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









