# STrade's development plan
This document describes how the project is build and what milestones are ahead of it.

## Brief idea
STrade is a simple trading simulator application.
The idea is simple:
 - Real-world data about stocks (and other market assets) in a
machine-readable format are fed to the application

 - The app parses them and allows the user to work with them
 - The user is given a virtual starting budget and is allowed to 
buy and sell those virtual assets

 - The app works in a simple loop:
   1. Market opens. Read users actions: buy, sell, manage → Confirm
   2. Market is closed. Process the requests and evaluate the inputs.
   3. Start the next day (cycle repeats)

## Technical implementation
Technical implementation will be updated alongside the project itself.

## TODO
Here is a list of features and their implementation status:

- Structs and data objects
  - [ ] Session information
  - [ ] User data
  - [ ] Day evaluation result