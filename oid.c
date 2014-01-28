#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <sys/stat.h>
#include <sysexits.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <err.h>

#define BUFFERSIZE	4096

char wantedbuf[BUFFERSIZE];
char actualbuf[BUFFERSIZE];

ssize_t wantedread, written;
ssize_t actualread = 0;

int fdin = 0;
int fdout;
off_t pos = 0;

unsigned long unchanged = 0, changed = 0, appended = 0;

int
main(int argc, char **argv) {
	ssize_t ret;

	if(argc != 2) {
		fprintf(stderr, "Usage: %s <file>\n", argv[0]);
		return EX_USAGE;
	}
	fdout = open(argv[1], O_RDWR);
	if(fdout == -1) {
		err(1, "open(%s, r+)", argv[1]);
	}
	while(1) {
		ret = read(fdin, wantedbuf, BUFFERSIZE);
		if(ret == -1) {
			err(1, "read(STDIN)");
		}
		if(ret == 0) {
			goto done;
		}
		wantedread = ret;

		actualread = 0;
		do {
			ret = read(fdout, actualbuf + actualread, wantedread - actualread);
			if(ret == -1) {
				err(1, "read(fdout)");
			}
			if(ret == 0) {
				if(lseek(fdout, pos, SEEK_SET) == -1) {
					err(1, "lseek(%s)", argv[1]);
				}
				goto extending;
			}
			actualread += ret;
		} while(wantedread > actualread);

		if(memcmp(wantedbuf, actualbuf, BUFFERSIZE) != 0) {
			changed++;
			written = 0;
			do {
				ret = pwrite(fdout, wantedbuf + written, wantedread - written, pos + written);
				if(ret == -1) {
					err(1, "pwrite(%s)", argv[1]);
				}
				assert(ret != 0);
				written += ret;
			} while(wantedread > written);
		} else {
			unchanged++;
		}
		pos += wantedread;
	}

extending:
	do {
		appended++;
		written = 0;
		do {
			ret = write(fdout, wantedbuf + written, wantedread);
			if(ret == -1) {
				err(1, "write(%s)", argv[1]);
			}
			written += ret;
		} while(wantedread > written);
		pos += written;

		ret = read(fdin, wantedbuf, BUFFERSIZE);
		if(ret == -1) {
			err(1, "read(STDIN)");
		}
		wantedread = ret;
	} while(ret != 0);

done:
	close(fdout);

	printf("Blocks unchanged: %lu; changed: %lu; appended: %lu\n", unchanged, changed, appended);

	return 0;
}
