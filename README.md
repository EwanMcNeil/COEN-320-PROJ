# COEN-320-PROJ

## How to Get Started

## STEP 1: Importing the Projects
1. In the Momentics IDE select File -> Import -> Existing Projects
2. Select the Project Source folder from the zip file and import both the 320_gui project and the COEN_320_PROJ project.
3. Do NOT build either project.

## STEP 2: Running the VM with Photon
1. Start a local QNX VM.
2. When prompted, enter "root" as the username
3. When logged enter the command "ph" to start the Photon Desktop Environment.
4. Note that the screen size may be small and you may encounter cursor issues. To resolve these, go to the "Display Settings" found either in the Start Menu which you can reach with Tab and the arrow keys or by right clicking the Desktop. Increase the Display resolution to something larger and set the cursor to "hardware".



### STEP 3: Uploading the Dataset Into the VM

1. From the main menu, select Window -> Show View -> Other
2. Select QNX Targets, then double-click Target File System Navigator
3. In the Target File System Navigator, select the "vm-target" (the program's target)
      
<p align="center">
  <img width="600" height="300" src="https://github.com/EwanMcNeil/COEN-320-PROJ/blob/master/screenshots/Step1/Frist.JPG">
</p>

4. Paste the CSV file into the /home directory
      
<p align="center">
  <img width="600" height="300" src="https://github.com/EwanMcNeil/COEN-320-PROJ/blob/master/screenshots/Step1/Second.png">
</p>

### STEP 4: Running the Projects
1. Create launch configurations for both projects that target the aforementioned QNX VM you have started.

2. Using "Run" (not Debug) launch the 320_gui project. You should see the GUI appear in the QNX VM.

3. Run the COEN_320_PROJ, it will begin to output results to the console.

4. On the GUI, click "run" to have these results visualized

      
<p align="center">
  <img width="600" height="400" src="https://github.com/EwanMcNeil/COEN-320-PROJ/blob/master/screenshots/Step2/Capture.JPG">
</p>
