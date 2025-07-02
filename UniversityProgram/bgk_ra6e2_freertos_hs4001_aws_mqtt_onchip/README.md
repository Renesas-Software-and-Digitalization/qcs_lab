
## Overview
This files contains changes required for running code Customizable Sensor Data to AWS Cloud application.

## Requirements
- Change sensor units (C to F or F to C)
- Change frequency of sensor update
- Modify the MQTT topic name as <sensor name>/<parameter>"
- Identify min, Max and averaging of sensor values. Send only when absolute change observed.
- Add widget (Gauge / box): Link to the confluence https://confluence.renesas.com/pages/viewpage.action?pageId=193805980#CustomizableSensorDatatoAWSCloudapplication-REQ-5:Addwidget(Gauge/box):

## Additional notes
main_thread_entry.c file includes the changes of Req-1 to Req-4. Copy the bgk_ra6e2_freertos\src\main_thread_entry.c file in to the QCS project repo path (<QCS_project>\src\main_thread_entry.c) then build.
