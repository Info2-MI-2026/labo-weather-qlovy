/**
 * Weather Analyzer.
 */
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MONTHS 12        // Months in a year
#define MAX_ENTRIES 512  // Maximum years in a file
#define MAX_LINE 80

typedef struct
{
    double temperature;
    double precipitations;
} WMonth;

typedef struct
{
    int year;
    WMonth months[MONTHS];
    double temperature;
    double precipitations;
} WYear;

typedef struct
{
    WYear years[MAX_ENTRIES];
    int start;
} WData;

typedef struct
{
    char *in_filename;
    char *out_filename;
    bool binary_output;
} Options;

void version(FILE *fp)
{
    fprintf(fp,
            "Version 0.0.1 "
            "Copyright(c) HEIG-VD\n");
}

void help(FILE *fp)
{
    fprintf(fp,
            "USAGE: ./weather-analyser [options] [filename]\n\n"
            "This program processes weather data issues from the internet. \n"
            "It reads from [filename] or if missing, \n"
            "directly from STDIN.\n\n"
            "The output format is CSV compliant.\n\n"
            "OPTIONS:\n\n"
            "    --version      Shows the version\n"
            "    --help         Shows this help dialog\n\n"
            "    --binary, -b   Output in binary mode, not CSV\n\n"
            "    -o<filename>   Write output on <filename>\n\n");
}

int collect_data(WData *data, FILE *fp) {
    char txt[MAX_LINE];
    while(fgets(txt, MAX_LINE, fp) != NULL){
        int n1, n2;
        double d1, d2;
        if (sscanf(txt, "%d\t%d\t%lf\t%lf", &n1, &n2, &d1, &d2) == 4){
            if (data->start == 0){
                data->start = n1;
            }
            data->years[n1 - data->start].year = n1;
            data->years[n1 - data->start].months[n2 - 1].temperature = d1;
            data->years[n1 - data->start].months[n2 - 1].precipitations = d2;
        }
    }
    return 0;
}

void process_data(WData *data) {
    for (int i=0 ; i<MAX_ENTRIES; i++){
        double sum_temp = 0;
        double sum_precip = 0;
        for (int j=0; j<MONTHS; j++){
            sum_temp += data->years[i].months[j].temperature;
            sum_precip += data->years[i].months[j].precipitations;
        }
        data->years[i].temperature = sum_temp/MONTHS;
        data->years[i].precipitations = sum_precip;
    }
}

void fprint_csv(FILE *fp, WData *data) {
    for (int i=0; i<MAX_ENTRIES; i++){
        if (data->years[i].year == 0) break;
        fprintf(fp, "%d,%.1lf,%.1lf\n", data->years[i].year, data->years[i].temperature, data->years[i].precipitations);
    }
}

void fprint_binary(FILE *fp, WData *data) {}

void process_arg(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--version") == 0)
        {
            version(stdout);
        }
        if (strcmp(argv[i], "--help") == 0)
        {
            help(stdout);
        }
    }
}

int main(int argc, char *argv[])
{
    process_arg(argc, argv);
    Options options;
    options.in_filename = "assets/weather-bern.txt";
    options.out_filename = "output.csv";
    WData wdata1 = {0};
    FILE* f = fopen(options.in_filename, "r");
    if (f == NULL) return 1;
    collect_data(&wdata1, f);
    fclose(f);
    process_data(&wdata1);

    f = fopen(options.out_filename, "w");
    if (f == NULL) return 1;
    fprint_csv(f, &wdata1);
    fclose(f);
}
