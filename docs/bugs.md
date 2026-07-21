Bugs/Nits found in the game
===

1. When in the pause menu, navigation to each menu option by means of arrow keys skips every other option focus
    - If the menu has options 1-6, then if option 1 is in focus and the user presses the down arrow once, then the focus jumps to option 3
    - The desired effect is to have the menu focus sequentially iterate through the menu option 1, 2, 3, 4, 5
    - This has the most impact on the save game menu options, there is no way to confirm "yes" to save a game

2. When the user uses the "w" key to move forward, while in 3rd person, on foot, perspective, the camera pans up towards the sky
    - The desired effect is for the camera to follow the player character when the "w" key is pressed
    - The camera should not pan up in this instance
