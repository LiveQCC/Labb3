#ifndef SAMPLER_H_
#define SAMPLER_H_

#include <stdint.h>

void initADC();
/**
 * Starts sampling on a given pin with a given frequency.
 * @param freq The sampling frequency in Hz
 */
void startSampling(int frequency);

/**
 * Stops the sampling process.
 */
void stopSampling();

/**
 * Computes the average frequency of the signal that has been (or is being) sampled.
 * @return The computed frequency in Hz
 */
float getFrequency(int frequency);

/**
 * Resets frequency detection without stopping the sampling process.
 */
void resetSampling();

#endif