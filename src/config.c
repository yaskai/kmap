#include <stdio.h>
#include <string.h>

#include "raylib.h"
#include "raymath.h"

#include "config.h"

// Read configuration options from provided file
void ConfigRead(Config *conf, char *path) {
	// Open file
	FILE *pF = fopen(path, "r");

	// Clear debug flags
	conf->debug_flags = 0;

	// Early out and error log if file path invalid
	if(!pF) {
		printf("ERROR: Could not open configuration file at: %s\n", path);	
		ConfigSetDefault(conf);
		return;
	}

	puts("Reading configuration file...");
	
	// Parse config file line by line
	char line[128];
	while(fgets(line, sizeof(line), pF)) 
		ConfigParseLine(conf, line);		
	
	// Close file, print debug messages
	fclose(pF);
	ConfigPrintValues(conf);
}

// Parse an individual line from config file
void ConfigParseLine(Config *conf, char *line) {
	// Ignore commented lines
	if(line[0] == '#') return;
	
	// Split value and option key
	char *eq = strchr(line, '='); 
	if(!eq) return;	

	*eq = '\0';
	char *key = line;
	char *val = eq + 1;

	if(streq(key, "window_width")) {
		// Window width:    
		// if auto option provided, set from monitor resolution
		if(streq(val, AUTO)) 
			conf->window_width = GetMonitorWidth(0);
		else 
			sscanf(val, "%d", &conf->window_width);

	} else if(streq(key, "window_height")) {
		// Window height:    
		// if auto option provided, set from monitor resolution
		if(streq(val, AUTO)) 
			conf->window_height = GetMonitorHeight(0);
		else
			sscanf(val, "%d", &conf->window_height);
		
	} else if(streq(key, "refresh_rate")) {
		// Refresh rate:
		// if auto option provided, set from monitor refresh rate
		if(streq(val, AUTO)) 
			conf->refresh_rate = GetMonitorRefreshRate(0);
		else 
			sscanf(val, "%f", &conf->refresh_rate);

	} else if(streq(key, "level_path")) {
		// Level Path:
		// for testing purposes
		if(streq(val, AUTO)) {
			char default_path[64] = "level.klf";
			memcpy(default_path, val, sizeof(default_path));

			char *n = strchr(default_path, '\n');
			*n = '\0';

			memcpy(conf->level_path, default_path, sizeof(default_path));
			
		} else {
			char custom_path[64];
			memcpy(custom_path, val, sizeof(custom_path));
			
			char *n = strchr(custom_path, '\n');
			*n = '\0';

			memcpy(conf->level_path, custom_path, sizeof(custom_path));
		}
	} 
}

// Set default config options 
void ConfigSetDefault(Config *conf) {
	*conf = (Config) {
		.refresh_rate  = CONFIG_DEFAULT_RR,
		.window_width  = CONFIG_DEFAULT_WW,
		.window_height = CONFIG_DEFAULT_WH,
		.debug_flags = 0,
	};

	ConfigPrintValues(conf);
}

// Print debug messages to shell
void ConfigPrintValues(Config *conf) {
	printf("resolution: %dx%d\n", conf->window_width, conf->window_height);
	printf("refresh rate: %f\n", conf->refresh_rate);
}

