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

sigset_t sigst;

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


void *fuelConsumption(void *);
void *engineSpeed(void *);
void *engineCoolantTemperature(void *);
void *currentGear(void *);
void *transmissionOilTemperature(void *);
void *vehicleSpeed(void *);
void *accelerationSpeedLongitudinal(void *);
void *indicationofbreakswitch(void *);

void *consumerThread(void *);



int start_periodic_timer(uint64_t offset, int period);
static void wait_next_activation(void);

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
	pthread_t th1, th2, th3;

	VALUES.fuelConsumption = 2.0;
	VALUES.engineSpeed = 0.0;
	VALUES.engineCoolantTemperature =0.0;
	VALUES.currentGear = 0.0;
	VALUES.transmissionOilTemperature = 0.0;
	VALUES.vehicleSpeed = 0.0;
	VALUES.accelerationSpeedLongitudinal = 0.0;
	VALUES.indicationofbreakswitch = 0.0;




	//not sure of differences between threads
	//and processes here
	sem_init(&structAccess, 0, 1);
	sem_init(&valueMutex, 0, 1);
	sem_init(&updateInterupt, 0, 0);
	sem_init(&printMutex,0,1);

	printf("Creating the consumer thread.\n");
	pthread_create(&th1,NULL, &consumerThread ,NULL);
	printf("Thread was created.\n");


	printf("Creating the FuelThread thread.\n");
	pthread_create(&th2,NULL, &fuelConsumption ,NULL);
	printf("Thread was created.\n");


	printf("Creating the engineSpeed thread.\n");
	pthread_create(&th2,NULL, &engineSpeed ,NULL);
	printf("Thread was created.\n");

	/* Hitting Ctrl+C sends a SIGTERM signal to the process. If the
	 * process receives this signal, it is killed along with all of
	 * its threads. We now use the pause() function to wait for
	 * the signal.
	 *
	 *
	 *
	 */

	 pthread_join(th1, NULL);
	 pthread_join(th2, NULL);
	 pthread_join(th3, NULL);
	 	pause();

	/* Hitting Ctrl+C will end the whole program, thus any lines
	 * after the pause() and the } will not be executed.
	 * If you want the main thread exit after the others threads
	 * finished, use pthread_join() as shown above.
	 * Have fun while exploring!
	 */

		return EXIT_SUCCESS;





}



void *fuelConsumption(void *empty)
{


	int count = 0;
	while (count < 4) {
		sem_wait(&printMutex);
		printf ("\n");
		printf ("updated Fuel\n");
		sem_post(&printMutex);

		sem_wait(&structAccess);
		count = count +1;
		VALUES.fuelConsumption = 2.0+count;
		sem_post(&structAccess);


		sem_post(&updateInterupt);


		//using sleep instead of timer for the current
		//moment
		sleep(1);


	}

	return NULL;
}



void *engineSpeed(void *empty){


		int count = 0;
		while (count < 4) {
			sem_wait(&printMutex);
			printf ("\n");
			printf ("updated EngineSpeed\n");
			printf ("\n");
			sem_post(&printMutex);

			sem_wait(&structAccess);
			count = count +1;
			VALUES.engineSpeed = 20*count*.8;
			sem_post(&structAccess);


			//wait_next_activation(); //wait for timer expiration
			sem_post(&updateInterupt);


			//using sleep instead of timer for the current
			//moment
			sleep(1);


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




//Timer code
static void wait_next_activation(void) {
	int dummy;
	/* suspend calling process until a signal is pending */
	sigwait(&sigst, &dummy);
}

int start_periodic_timer(uint64_t offset, int period) {
	struct itimerspec timer_spec;
	struct sigevent sigev;
	timer_t timer;
	const int signal = SIGALRM;
	int res;

	/* set timer parameters */
	timer_spec.it_value.tv_sec = offset / ONE_MILLION;
	timer_spec.it_value.tv_nsec = (offset % ONE_MILLION) * ONE_THOUSAND;
	timer_spec.it_interval.tv_sec = period / ONE_MILLION;
	timer_spec.it_interval.tv_nsec = (period % ONE_MILLION) * ONE_THOUSAND;

	sigemptyset(&sigst); // initialize a signal set
	sigaddset(&sigst, signal); // add SIGALRM to the signal set
	sigprocmask(SIG_BLOCK, &sigst, NULL); //block the signal

	/* set the signal event a timer expiration */
	memset(&sigev, 0, sizeof(struct sigevent));
	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = signal;

	/* create timer */
	res = timer_create(CLOCK_MONOTONIC, &sigev, &timer);

	if (res < 0) {

		printf("timer end /n");
		perror("Timer Create");
		exit(-1);
	}


	/* activate the timer */
	return timer_settime(timer, 0, &timer_spec, NULL);
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








