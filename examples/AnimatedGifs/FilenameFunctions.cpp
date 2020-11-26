/*
 * Animated GIFs Display Code for SmartMatrix and 32x32 RGB LED Panels
 *
 * This file contains code to enumerate and select animated GIF files by name
 *
 * Written by: Craig A. Lindley
 */

#include "FilenameFunctions.h"

#include <SD.h>

File file;

int numberOfFiles;

bool fileSeekCallback(unsigned long position) {
    return file.seek(position);
}

unsigned long filePositionCallback(void) {
    return file.position();
}

int fileReadCallback(void) {
    return file.read();
}

int fileReadBlockCallback(void * buffer, int numberOfBytes) {
    return file.read((uint8_t*)buffer, numberOfBytes);
}

int fileSizeCallback(void) {
    return file.size();
}

int initFileSystem(int chipSelectPin) {
    // initialize the SD card at full speed
    if (chipSelectPin >= 0) {
        pinMode(chipSelectPin, OUTPUT);
    }
    if (!SD.begin(chipSelectPin))
        return -1;
    return 0;
}

bool isAnimationFile(const char filename []) {
    String filenameString(filename);

#if defined(ESP32)
    // ESP32 filename includes the full path, so need to remove the path before looking at the filename
    int pathindex = filenameString.lastIndexOf("/");
    if(pathindex >= 0)
        filenameString.remove(0, pathindex + 1);
#endif

    if ((filenameString[0] == '_') || (filenameString[0] == '~') || (filenameString[0] == '.')) {
        return false;
    }

    filenameString.toUpperCase();
    if (filenameString.endsWith(".GIF") != 1)
        return false;

    return true;
}

// Enumerate and possibly display the animated GIF filenames in GIFS directory
int enumerateGIFFiles(const char *directoryName, bool displayFilenames) {

    numberOfFiles = 0;

    File directory = SD.open(directoryName);
    File file;

    if (!directory) {
        return -1;
    }

    while (file = directory.openNextFile()) {
        if (isAnimationFile(file.name())) {
            numberOfFiles++;
            if (displayFilenames) {
                Serial.print(numberOfFiles);
                Serial.print(":");
                Serial.print(file.name());
                Serial.print("    size:");
                Serial.println(file.size());
            }
        } else if (displayFilenames) {
            Serial.println(file.name());
        }

        file.close();
    }

    //    file.close();
    directory.close();

    return numberOfFiles;
}

// Get the full path/filename of the GIF file with specified index
void getGIFFilenameByIndex(const char *directoryName, int index, char *pnBuffer) {


    // Make sure index is in range
    if ((index < 0) || (index >= numberOfFiles))
        return;

    File directory = SD.open(directoryName);
    if (!directory)
        return;

    while ((index >= 0)) {
        file = directory.openNextFile();
        if (!file) break;

        if (isAnimationFile(file.name())) {
            index--;

            // Copy the directory name into the pathname buffer			
            strcpy(pnBuffer, directoryName);
			
			//ESP32 SD Library includes the full path name in the filename, so no need to add the directory name
#if defined(ESP32)
            pnBuffer[0] = 0;
#else
            int len = strlen(pnBuffer);
            if (len == 0 || pnBuffer[len - 1] != '/') strcat(pnBuffer, "/");
#endif

            // Append the filename to the pathname
            strcat(pnBuffer, file.name());
        }

        file.close();
    }

    file.close();
    directory.close();
}

int openGifFilenameByIndex(const char *directoryName, int index) {
    char pathname[255];

    getGIFFilenameByIndex(directoryName, index, pathname);
    
    Serial.print("Pathname: ");
    Serial.println(pathname);

    if(file)
        file.close();

    // Attempt to open the file for reading
    file = SD.open(pathname);
    if (!file) {
        Serial.println("Error opening GIF file");
        return -1;
    }

    return 0;
}


// Return a random animated gif path/filename from the specified directory
void chooseRandomGIFFilename(const char *directoryName, char *pnBuffer) {

    int index = random(numberOfFiles);
    getGIFFilenameByIndex(directoryName, index, pnBuffer);
}
