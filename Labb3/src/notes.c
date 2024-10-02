#include "notes.h"
#include <math.h>

void freq2note(float freq, char *stringbuff) {
    if (freq < 0.0) {
        stringbuff[0] = '-';
        stringbuff[1] = '-';
        stringbuff[2] = '\0';
        return;
    }
    if (freq == 0.0) {
        stringbuff[0] = 'R';
        stringbuff[1] = 'E';
        stringbuff[2] = '\0';
        return;
    }

    // Frequencies of open strings in standard tuning (E2, A2, D3, G3, B3, E4)
    float guitarFrequencies[] = {82.41, 110.0, 146.83, 196.0, 246.94, 329.63};
    char *guitarNotes[] = {"E2", "A2", "D3", "G3", "B3", "E4"};

    // Find the closest standard tuning note to the given frequency
    float minDiff = fabs(freq - guitarFrequencies[0]);
    int closestNoteIndex = 0;
    for (int i = 1; i < 6; ++i) {
        float diff = fabs(freq - guitarFrequencies[i]);
        if (diff < minDiff) {
            minDiff = diff;
            closestNoteIndex = i;
        }
    }

    // Copy the closest note to the string buffer
    stringbuff[0] = guitarNotes[closestNoteIndex][0];
    stringbuff[1] = guitarNotes[closestNoteIndex][1];
    stringbuff[2] = guitarNotes[closestNoteIndex][2] ? guitarNotes[closestNoteIndex][2] : '\0';
    stringbuff[3] = '\0';
}