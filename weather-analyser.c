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
#define MAX_LENGTH 40

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
    char in_filename[MAX_LENGTH];
    char out_filename_csv[MAX_LENGTH];
    char out_filename_bin[MAX_LENGTH];
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
    fprintf(fp, "year;temperatures;precipitations\n");
    for (int i=0; i<MAX_ENTRIES; i++){
        if (data->years[i].year == 0) break;
        fprintf(fp, "%d,%.1lf,%.1lf\n", data->years[i].year, data->years[i].temperature, data->years[i].precipitations);
    }
}

void fprint_binary(FILE *fp, WData *data) {
    char* header = "WEATHER";
    const int size = sizeof(header);
    fwrite(header, size, 1, fp);
    for(int i=0; i<MAX_ENTRIES; i++){
        if (data->years[i].year == 0) break;
        fwrite(&(data->years[i].year), 4, 1, fp);
        int temp = data->years[i].temperature * 10;
        fwrite(&temp, 4, 1, fp);
        int precip = data->years[i].precipitations * 100;
        fwrite(&precip, 4, 1, fp);
    }
}

void process_arg(int argc, char *argv[], Options* options)
{
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i], "--version") == 0){
            version(stdout);
        }else if (strcmp(argv[i], "--help") == 0){
            help(stdout);
        }else if (strcmp(argv[i], "--binary") == 0 || strcmp(argv[i], "-b") == 0){
            options->binary_output = true;
        }else if(sscanf(argv[i], "-o%s", options->out_filename_csv) == 1){
        }else{
            sscanf(argv[i], "%s", options->in_filename);
        }
    }
    if (options->binary_output){
        strcpy(options->out_filename_bin, options->out_filename_csv);
    }
}

int main(int argc, char *argv[])
{
    Options options = {.in_filename = "assets/weather-bern.txt", 
        .out_filename_csv = "out.csv", .out_filename_bin = "out.bin",
        .binary_output = false};

    process_arg(argc, argv, &options);
    
    WData wdata1 = {0};
    FILE* f = fopen(options.in_filename, "r");
    if (f == NULL) return 1;
    collect_data(&wdata1, f);
    fclose(f);
    process_data(&wdata1);

    if (options.binary_output){
        f = fopen(options.out_filename_bin, "w");
        if (f == NULL) return 2;
        fprint_binary(f, &wdata1);
        fclose(f);
    }else{
        f = fopen(options.out_filename_csv, "w");
        if (f == NULL) return 3;
        fprint_csv(f, &wdata1);
        fclose(f);
    }

    return 0;
}
