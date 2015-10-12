#ifndef RES_PATH_H
#define RES_PATH_H

#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

/*
 * Get the resource path for resources located in res/subDir
 * It's assumed the project directory is structured like:
 * bin/
 *  the executable
 * res/
 *  Lesson1/
 *  Lesson2/
 *
 * Paths returned will be Lessons/res/subDir
 */
char* 
sdlGetResourcePath(char* subDir = 0) {
	// We need to choose the path separator properly based on which
	// platform we're running on, since Windows uses a different
	// separator than most systems
#ifdef _WIN32
 	char* PATH_SEP = "\\";
#else
	char* PATH_SEP = "/";
#endif
	// This will hold the base resource path: Lessons/res/
	// We give it static lifetime so that we'll only need to call
	// SDL_GetBasePath once to get the executable path
	static char* baseRes;
	if (baseRes == 0) {
		// SDL_GetBasePath will return NULL if something went wrong in 
		// retrieving the path
		char *basePath = SDL_GetBasePath();
		if (!basePath) {
			fprintf(stderr, "Error getting resource path: %s\n",  SDL_GetError());
			return 0;
		}
		// We replace the last build/ with build/ to get the the resource path
		char* pos = strstr(basePath, "build");
		if (!pos) {
			fprintf(stderr, "Error getting build folder path\n");
			return 0;
		}
		int baseResSize = strlen(basePath) - strlen("build") + strlen("res") + 1;
		baseRes = (char*) malloc(baseResSize);
		if (!baseRes) {
      		fprintf(stderr,"malloc failed!\n");
			return 0;
		}
		baseRes[0] = 0;
		strncat(baseRes, basePath, pos - basePath); 
		strcat(baseRes, "res"); 
		strcat(baseRes, PATH_SEP);

		SDL_free(basePath);
	}
	//If we want a specific subdirectory path in the resource directory
	//append it to the base path. This would be something like Lessons/res/Lesson0
	char* result;
	if (subDir) {
		int resultSize = strlen(baseRes) + strlen(subDir) + 2;
		result = (char*) malloc(resultSize);
		if (!result) {
      		fprintf(stderr,"malloc failed!\n");
			return 0;
		}
		strcpy(result, baseRes); 
		strcat(result, subDir); 
		strcat(baseRes, PATH_SEP);

	} else {
		int resultSize = strlen(baseRes) + 1;
		result = (char*) malloc(resultSize);
		if (!result) {
      		fprintf(stderr,"malloc failed!\n");
			return 0;
		}
		result[resultSize - 1] = 0;
		memcpy(result, baseRes, resultSize - 1); 
	}
	return result;
}

#endif