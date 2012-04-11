#ifndef SUBPROCESS_HPP
#define SUBPROCESS_HPP

#include <sys/types.h>
//#include <stdio.h>
//#include "bufreader.h"


#include <string>
#include <vector>


class Subprocess {
	public:
		enum Status { BeforeExec, Exec, AfterExec, Invalid };

		// Create a Subprocess object for a binary
		Subprocess(std::string binary, std::vector<std::string> args);
		// Free memory associated with a subproc
		~Subprocess();

		// Actually execute the configured binary
		int run();
		// Attempts to update the status and returns the new one
		Status status();
		// Send the SIGKILL signal to the running subprocess
		int kill();

		// Return a write-mode FILE * hooked to the stdin of a subprocess
		//FILE *subprocess_wfile();
		// Returns a valid line read from stdout, or NULL if nothing was available
		//char *subprocess_read();

	protected:
		std::string m_binary;
		std::vector<std::string> m_args;

		int m_pipe[2];
		Status m_status;
		pid_t m_spid;
		int m_value;

		//BufReader *br;
};

#endif // SUBPROCESS_HPP
