# Power Source

This folder details experiments and changes made to the power supply of the physiotherapist wearable.  This is neccessary to ensure physiotherapists charge the device to ensure it does not run out of charge, and to fully utilise the capacity of the battery.


## 03/09/22
#

## **Resistors**

The aim of the experiment was to see if using potential difference was a feasible way of estimating battery capacity. A multimeter was used to get the true value of the potential difference and that value is compared to the analogRead when resistors are applied in a potential divicer circuit. This is the circuit used:

    VCC - R1 * R1 - GND 
    
with * being where the analogRead is taken from. The reason for this is because our MCU is 3.3V logic level. As the battery has a maximum voltage level of 4.2V, this is to ensure the logic level is not exceeded.

From some preliminary experiements, there was quite abit of inaccuracy. Firstly, with higher resistances, the ADC within the MCU is more inaccurate as it works by storing charge. Hence, the low current decreases accuracy.

However at lower resistances, alot of voltage is lost to power loss due to the current at the branch. As such there was a consistent offset.

| Resistance  | Multimeter Voltage | Actual Volage |
| ----------- | -----------        | -----------   | 
| 270 &Omega; | 2.59V              | 2.74V         | 
| 1 M&Omega;  | 2.45V              | 2.08V - 2.74V |
| 100 K&Omega;| 2.58V              | 2.40V         |

As noticed the resistance did not seem to affect the offset heavily. However, a higher resistance had a very sigificant amount of fluctuation


## **Voltage Discharge**

A DC Motor Was used to discharge a fully charged LION battery and its potential difference was measured using 100K&Omega; resistors. The potential difference was sampled every 1s using the M5 Atom's ADC pin, and recorded on to a CSV file.

![Electrical Setup](SRC/AnalogRead_DC.jpg "Electrical Setup")

![Voltage Graph](SRC/Graph.png "Graph of Voltage Against Time")

The voltage graph had alot of fluctuation. Although granted that this could be due to signal noise, but it might mean that we might need to implement some signal filtering techniques when using this method to estimate battery capacity. However, from this graph, it does not look like a very reliable method and we might need to consider using other methods to get the capacity of the battery.

Some possible Alternative Ways coule be using breakout boards that monitor the capacity of the battery.


## *Possible Links*

[Single Li-Ion and LiPoly Battery Power Meter](https://www.adafruit.com/product/5383)

[Adafruit LC709203F LiPoly / LiIon Fuel Gauge and Battery Monitor](https://www.adafruit.com/product/4712)

[Adafruit MAX17048 LiPoly / LiIon Fuel Gauge and Battery Monitor](https://www.adafruit.com/product/5580)