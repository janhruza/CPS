//
// Created by jan on 05/10/2026.
//

// commands.h
// Representing various application commands - for a user to use.

#ifndef NUCLEARPLANT_COMMANDS_H
#define NUCLEARPLANT_COMMANDS_H

#define ANSI_CLEAR     "\033[2J\033[H"

typedef enum tagResult {
    SUCCESS = 0,    // Result was a success.
    ERROR_GENERIC,  // Result was a failure.
    FATAL = 0xDEAD  // Special fatal case (should be the last in the list).
} RESULT;

/**
 * Representing the help command.
 * @return Operation result.
 */
RESULT CmdHelp(void);

/**
 * Representing the exit command.
 * @return Operation result.
 */
RESULT CmdQuit(void);

/**
 * Representing the version command.
 * @return Operation result.
 */
RESULT CmdVersion(void);

/**
 * Clears the console screen and its buffer. It also sets the cursor to its initial position.
 * @return Operation result.
 */
RESULT CmdClear(void);

#endif //NUCLEARPLANT_COMMANDS_H
