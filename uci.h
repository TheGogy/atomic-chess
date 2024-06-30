#pragma once

#ifndef UCI_H
#define UCI_H

#define ENGINE_NAME "Atom"
#define ENGINE_AUTHOR "TheGogy"

#ifdef ATOMIC
#define ENGINE_VERSION "Atomic"
#else
#define ENGINE_VERSION "Standard"
#endif // ATOMIC


#define STARTPOS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

void handle_uci();

#endif // !UCI_H
