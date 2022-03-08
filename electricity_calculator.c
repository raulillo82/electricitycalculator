#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define OFF_PEAK_PRICE 0.089 /*In euros/kWh*/
#define PEAK_PRICE 0.16 /*In euros/kWh*/
#define POWER_PRICE 38.043585 /*In euros/kW·year*/
#define PEAK_TIME_WINTER_START 12
#define PEAK_TIME_SUMMER_START 13
#define PEAK_TIME_DURATION 10
#define SUMMER_SEASON_START 4 /*Month in which summer season starts*/
#define SUMMER_SEASON_END 10 /*Month in which summer season ends*/
#define ELECTRICITY_TAX 0.0511269632
#define VAT 0.21
#define MAX_STRING_LENGTH 10 

void get_data (double *start_time, double *duration_hours, double *energy, struct tm *local_time); 
double get_cost (double start_time, double duration_hours, double energy, struct tm *local_time); 
void print_cost (double cost);
void calculate_bill ();
void menu ();
void read_string (char *string);

void get_data (double *start_time, double *duration_hours, double *energy, struct tm *local_time){ 
	//int hour, minute;
	int i = 0, check = 0, start_hour = 0, start_minute = 0;
	char *hour_string, *minute_string, *duration_string, *energy_string, *pos = NULL;

	/*String initialization*/
	hour_string = malloc (MAX_STRING_LENGTH * sizeof(char));
	minute_string = malloc (MAX_STRING_LENGTH * sizeof(char));
	duration_string = malloc (MAX_STRING_LENGTH * sizeof(char));
	energy_string = malloc (MAX_STRING_LENGTH * sizeof(char));

	printf ("\nPlease enter the time in which consume started.\n");
	printf ("Introduce the hour (0-23) or press enter for using the time now: ");
	/*scanf ("%s", hour_string);*/
	
	read_string (hour_string);
	/*fgets (hour_string, MAX_STRING_LENGTH, stdin);

	if ((pos = strchr(hour_string, '\n')) != NULL)
		*pos = '\0';*/


	if (!strcmp ("", hour_string)){
		printf ("\nDefault string\n");
		start_hour = local_time->tm_hour;
		start_minute = local_time->tm_min;
	}

	else{
		printf ("\nEntering time manually\n");
		printf ("%s\n", hour_string);
		start_hour = atoi (hour_string);
		//*start_hour = strtol (hour_string, NULL, 10);
		//printf ("%d", *start_hour);
		printf ("\nIntroduce the minute (0-59): ");
		//scanf ("%d", &start_minute);
		/*fgets (hour_string, MAX_STRING_LENGTH, stdin);
		if ((pos = strchr(hour_string, '\n')) != NULL)
			*pos = '\0';*/
		read_string (minute_string);
		start_minute = atoi (minute_string);
	}
	*start_time = start_hour + start_minute / 60;

	printf ("\nPlease enter duration of consume in minutes (decimals possible)\n");
	printf ("Introduce number of minutes or press enter if you know the full amount of kWh: ");
	
	read_string (duration_string);
	/*fgets (duration_string, MAX_STRING_LENGTH, stdin);

	if ((pos = strchr(duration_string, '\n')) != NULL)
		*pos = '\0';*/

	if (!strcmp ("", duration_string)){
		printf ("\nDefault duration, one hour will be assumed\n");
		*duration_hours = 1.0;
	}
	else{
		*duration_hours = atof (duration_string) / 60;
		if (*start_time + *duration_hours >= 24.0){
			printf ("\nWarning, program does not compute throught another day, calculating consume only till midnight of today\n");
			*duration_hours = 24 - *start_time;
			printf ("Total duration set to %f hours\n", *duration_hours);
		}
	}

	printf ("\nPlease introduce the consume of the device to measure in kWatts, or the energy used in kWh if you used the default in the previous option\n");
	printf ("Introduce consume (kW) or energy (kWh): ");
	//scanf ("%f", energy);
	read_string (energy_string);
	*energy = atof (energy_string);
	//printf ("Energia = %f\nDuracion = %f\n", *energy, *duration_hours);
	*energy *= *duration_hours;

	printf ("\nEnergy used is: %f kWh\n", *energy);
	
	/*Free memory*/
	free (hour_string);
	free (minute_string);
	free (duration_string);
	free (energy_string);
}

double get_cost (double start_time, double duration_hours, double energy, struct tm *local_time){ 
	double cost = 0;
	int peak_time_start;

	/*Find out whether it's summertime or not, in order to choose one time or other for the peak time start*/
	if (local_time->tm_isdst)
		peak_time_start = PEAK_TIME_SUMMER_START;
	else
		peak_time_start = PEAK_TIME_WINTER_START;

	if (start_time < peak_time_start){
		if (start_time + duration_hours <= peak_time_start)
			cost = OFF_PEAK_PRICE * energy;
		else{
			double off_peak_duration = peak_time_start - start_time;
			cost = off_peak_duration * OFF_PEAK_PRICE * energy / duration_hours;
			//printf ("\n Coste periodo valle: %f euros en %f horas\n", cost, off_peak_duration);
			if (start_time + duration_hours <= peak_time_start + PEAK_TIME_DURATION){
				double peak_duration = duration_hours - off_peak_duration;
				cost += peak_duration * PEAK_PRICE * energy / duration_hours;
				//printf ("\n Coste acumulado con periodo punta: %f euros en %f horas\n", cost, peak_duration);
			}
			else
				cost += (PEAK_TIME_DURATION * PEAK_PRICE + (duration_hours - off_peak_duration - PEAK_TIME_DURATION) * OFF_PEAK_PRICE) * energy / duration_hours;
		}
	}
	else{
		if (start_time < peak_time_start + PEAK_TIME_DURATION){
			if (start_time + duration_hours <= peak_time_start + PEAK_TIME_DURATION)
				cost = PEAK_PRICE * energy;
			else{
				double peak_duration = peak_time_start + PEAK_TIME_DURATION - start_time;
				cost = (peak_duration * PEAK_PRICE + (duration_hours - peak_duration) * OFF_PEAK_PRICE) * energy / duration_hours;
			}
		}
		else
			cost = OFF_PEAK_PRICE * energy; 
	}
			
	
	return cost;
}	

void print_cost (double cost){
	printf ("Total cost calculated before tax is: %f euros\n", cost);
	printf ("Total cost calculated after electricity tax is: %f euros\n", cost*(1 + ELECTRICITY_TAX));
	printf ("Total cost calculated after electricity tax and VAT is: %f euros\n", cost*(1 + ELECTRICITY_TAX)*(1 + VAT));
}
void calculate_bill (){
	double p1, p2, power, cost;
	char *peak_energy, *off_peak_energy, *power_string;

	peak_energy = malloc (MAX_STRING_LENGTH * sizeof(char));
	off_peak_energy = malloc (MAX_STRING_LENGTH * sizeof(char));
	power_string = malloc (MAX_STRING_LENGTH * sizeof(char));

	printf ("\nIntroduce peak energy kWh: ");
	//scanf ("%f", &p1);
	read_string (peak_energy);
	p1 = atof (peak_energy);

	printf ("\nIntroduce off-peak energy kWh: ");
	//scanf ("%f", &p2);
	read_string (off_peak_energy);
	p2 = atof (off_peak_energy);
	
	printf ("\nIntroduce power in contract in kWh: ");
	//scanf ("%f", &power);
	read_string (power_string);
	power = atof (power_string);

	cost = power * POWER_PRICE * 30 / 365 + p1 * PEAK_PRICE + p2 * OFF_PEAK_PRICE;

	print_cost (cost);
	
	free (peak_energy);
	free (off_peak_energy);
	free (power_string);
}

void read_string (char *string){
	char *pos = NULL;
	fgets (string, MAX_STRING_LENGTH, stdin);
	if ((pos = strchr(string, '\n')) != NULL)
		*pos = '\0';
}

void menu (){
	int option;
	char *option_string, *pos = NULL;
	//char dummy;
	double start_time, duration_hours, energy, cost;
	time_t current_time = (time_t)NULL;
	struct tm *local_time;

	option_string = malloc (MAX_STRING_LENGTH * sizeof(char));
	do{
		//printf("\e[1;1H\e[2J");
		printf ("1- Calculate cost of usage of an electrical device in a given time\n");
		printf ("2- Calculate electricity bill cost\n");
		printf ("3- Exit");
		printf ("\nPlease enter an option: ");
		//scanf ("%c", &option);
		//option = getc(stdin);

		//scanf ("%d", &option);

		read_string (option_string);
		/*fgets (option_string, MAX_STRING_LENGTH, stdin);

		if ((pos = strchr(option_string, '\n')) != NULL)
			*pos = '\0';*/

		option = atoi (option_string);


		switch (option){
			case 1:
				current_time = time (NULL);
				local_time = gmtime (&current_time);

				get_data (&start_time, &duration_hours, &energy, local_time);
				cost = get_cost (start_time, duration_hours, energy, local_time);
				print_cost (cost);
				//scanf ("%c", &dummy);

				break;
			case 2:
				calculate_bill ();
				//scanf ("%c", &dummy);
				break;
			case 3:
				break;
			default:
				break;
		}
	} while (option != 3);
	free (option_string);
}
int main (){
	/*double start_time, duration_hours, energy, cost;
	time_t current_time = (time_t)NULL;
	struct tm *local_time;

	current_time = time (NULL);
	local_time = gmtime (&current_time);

	get_data (&start_time, &duration_hours, &energy, local_time);
	cost = get_cost (start_time, duration_hours, energy, local_time);
	print_cost (cost);*/

	menu ();
	return 0;
}
