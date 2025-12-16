#include <stdint.h>

#ifndef CONFIG_H_
#define CONFIG_H_

// Configuration defaults, revert to these values if config file not found
// Default window width, height and refresh rate
#define CONFIG_DEFAULT_WW	1920
#define CONFIG_DEFAULT_WH	1080
#define CONFIG_DEFAULT_RR	  60

#define AUTO "auto"
#define streq(a, b) (strcmp((a), (b)) == 0)

#define CONFIG_PATH	"options.txt"

enum TARGET_PLATFORMS {
	PLATFORM_LINUX = 0,
	PLATFORM_WIN64 = 1,
	PLATFORM_MACOS = 2
};

// **
// Change when compiling for user
#define TARGET_PLATFORM (PLATFORM_LINUX)

typedef struct {
	char level_path[128];

	float refresh_rate;

	unsigned int window_width;
	unsigned int window_height;

	uint8_t debug_flags;
} Config;

void ConfigRead(Config *conf, char *path);
void ConfigParseLine(Config *conf, char *line);
void ConfigSetDefault(Config *conf);
void ConfigPrintValues(Config *conf);

#endif // !CONFIG_H_
