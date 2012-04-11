#include "subprocess.hpp"
using std::string;
using std::vector;

#include "util.hpp"

#include <sys/wait.h>
#include <unistd.h>

static int unpipe(int pipe[2]);
static int unpipe(int pipe[2]) {
	int f1 = close(pipe[0]), f2 = close(pipe[1]);
	return (f1 || f2);
}

Subprocess::Subprocess(string binary, vector<string> args) : // {{{
		m_binary(binary), m_args(args), m_pipe(), m_status(Subprocess::Invalid),
		m_spid(0), m_value(-1) {
	/*
	subproc->argv[0] = subproc->binary;
	subproc->argv[argc + 1] = NULL;
	for(int i = 0; i < argc; ++i) {
		subproc->argv[i + 1] = strdup(argv[i]);
		if(subproc->argv[i + 1] == NULL) {
			subprocess_free(subproc);
			return NULL;
		}
	}
	*/
} // }}}
Subprocess::~Subprocess() { // {{{
	if(this->m_status == Subprocess::Exec)
		this->kill();
} // }}}

int Subprocess::run() {
	// if we're not in the before exec phase, abort
	if(this->m_status != Subprocess::BeforeExec)
		return -1;

	// if the binary doesn't exist, abort
	if(!util::file::exists(this->m_binary)) {
		this->m_status = Subprocess::Invalid;
		return -1;
	}

	int left[2] = { 0 }, right[2] = { 0 };
	// if we can't create the left pipe, abort
	int fail = pipe(left);
	if(fail)
		return fail;
	// if we can't create the right pipe, abort
	fail = pipe(right);
	if(fail) {
		unpipe(left);
		return fail;
	}

	// if we can't fork, abort
	this->m_spid = fork();
	if(this->m_spid == -1) {
		unpipe(left);
		unpipe(right);
		return -2;
	}

	// if we're the child, execv the binary
	if(this->m_spid == 0) {
		dup2(left[0], 0);
		unpipe(left);

		dup2(right[1], 1);
		unpipe(right);

		// build subprocess argv
		const char **argv = new const char*[this->m_args.size() + 2];
		argv[0] = this->m_binary.c_str();
		for(unsigned i = 0; i < this->m_args.size(); ++i)
			argv[i + 1] = this->m_args[i].c_str();
		argv[this->m_args.size() + 1] = NULL;

		// TODO: man this is a strange conversion
		execv(this->m_binary.c_str(), (char* const*)argv);
		return -99;
	}

	// if we're main, close uneeded ends and copy fds
	close(left[0]);
	close(right[1]);
	this->m_pipe[0] = right[0];
	this->m_pipe[1] = left[1];

	/*
	// TODO: create above, setup now?
	this->m_br = bufreader_create(this->m_pipe[0], "\n", 4096);
	if(this->m_br == NULL) {
		fprintf(stderr, "subprocess_run: couldn't create bufreader\n");
		// TODO: better handling
		return -80;
	}
	*/

	this->m_status = Subprocess::Exec;
	return 0;
}

Subprocess::Status Subprocess::status() {
	if(this->m_status != Subprocess::Exec)
		return this->m_status;
	pid_t pid = waitpid(this->m_spid, &this->m_value, WNOHANG);
	if(pid == this->m_spid)
		this->m_status = Subprocess::AfterExec;
	return this->m_status;
}
int Subprocess::kill() {
	if(this->m_status != Subprocess::Exec)
		return -1;
	// TODO: this scoping is really fun :)
	return ::kill(this->m_spid, SIGKILL);
}

/*
FILE *Subprocess::wfile(Subprocess *subproc) {
	return fdopen(subproc->pipe[1], "w");
}
char *Subprocess::read(Subprocess *subproc) {
	return bufreader_read(subproc->br);
}
*/

