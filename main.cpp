#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

#include "parser.h"
#include "execute.h"
#include "terminal.h"


/*TODO:
 *  - arrow key selection
 *  - command not found
 *  - ctrl <key> actions <- implementing gnu readline
 *  - history (searching in history, start typing then press up and down to do backward and forward searches through my history for commands starting with what I've just typed)
 *  -
 */
    // TODO (Pigeon Command Arguments):
    // - In 'pigeon run <alias> [args...]': replace positional placeholders ($1, $2, ...) in the stored template.
    // - Re-feed the expanded string through parser() before invoking execute().
    // TODO (Shell Robustness & Error Handling):
    // - main(): Check std::cin stream state after getline(); terminate loop cleanly on EOF (Ctrl+D).
    // - execute(): Close open pipe ends in the parent process if fork() fails (returns -1) to prevent descriptor leaks.
    // - execute(): Guard against empty commands[0].args before indexing args[0].

    // TODO (Signals & Process Management):
    // - Handle SIGINT (Ctrl+C) so it interrupts child execution rather than killing the shell.
    // - Inspect exit status in waitpid() via WIFEXITED and WEXITSTATUS macros.

int main() {
    while (true) {
        std::cout << ">" << std::flush;
        // std::getline(std::cin, raw_input_data);
        terminal_raw_mode terminal;
        std::string raw_input_data = terminal.read_line();
        std::vector<command> commands;

        try {
            commands = parser(raw_input_data);
            if (commands.empty()) {
                continue;
            }
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            continue;
        }
        if (!execute(commands)) {
            break;
        }
    }

    return 0;
}

