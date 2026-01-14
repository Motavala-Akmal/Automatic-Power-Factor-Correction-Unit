## Working Principle of Automatic Power Factor Correction (APFC) System
## 1. Overview
      Many AC loads such as induction motors and transformers are inductive in nature, which leads to a low power factor due to the presence of reactive power. 
      A poor power factor increases line current, causes additional losses, and reduces overall system efficiency.
      This project implements an Automatic Power Factor Correction (APFC) system that continuously monitors the electrical parameters of an AC load and improves the power factor whenever it drops below a set limit. 
      An ESP32 is used as the main controller, while a PZEM-004T energy monitoring module provides real-time measurements of voltage, current, power, energy, and power factor.
      Based on the measured power factor, the controller generates alerts and controls a relay-operated capacitor bank to compensate for reactive power.
      The system operates automatically, adapts to changing load conditions, and provides both local display and web-based monitoring.   
