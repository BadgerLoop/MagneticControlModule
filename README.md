# MagneticControlModule
For the Magnetic/Motor Control Max32s.

This project must be compiled using MPLAB and a PICkit 3.

It can be a challenge to use git with MPLAB. A good way to open this project would be to create a new project in MPLAB, right click the project in the Projects tab and navigate to versioning. Click initialize new git repository. Then, right click the project again and there should be a git menu entry towards the bottom, navigate to it and click add. Then right click the project again and do commit, any message will do (i.e. "Initial commit."). These steps will allow us to do a pull and completely rebase from the source of our choosing. Navigate to the git menu again and select pull (can be found inside 'remote'). We will specify the repository location as https://github.com/BadgerLoop/MagneticControlModule.git. Name the remote 'origin' and enter your gitHub username and password (contact Vaughn Kottler on Slack if you don't have permission to pull). Click next, select the master branch and click next again. This should work.

If this DOESN'T work there are workarounds. You can always download the repository, unzip it somewhere, and then tell MPLAB to open the project by navigating to it. To turn it back in to a git repository you will have to right click the project, and initialize a new git repository from the versioning menu. Then you will have to add, do an initial commit, and then you should be able to push and pull.
