// file globals_BM.h
// SPDX-License-Identifier: GPL-3.0-or-later

/***
    BISMON 
    Copyright © 2018 - 2022 CEA (Commissariat à l'énergie atomique et aux énergies alternatives)
    contributed by Basile Starynkevitch (working at CEA, LIST, France)
    <basile@starynkevitch.net> or <basile.starynkevitch@cea.fr>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***/

#ifndef GLOBALS_BM_INCLUDED
#define GLOBALS_BM_INCLUDED
// from generated _timestamp.c
extern const char bismon_timestamp[];
extern const unsigned long bismon_timelong;
extern const char bismon_lastgitcommit[];
extern const char bismon_lastgittag[];
extern const char bismon_checksum[];
extern const char bismon_directory[];
extern const char bismon_gnumakefile[];
extern const char bismon_gitid[];
extern const char bismon_shortgitid[];

// from generated _bm_allconsts.c
extern const int bmnbconsts;
extern void **bmconstaddrs[];
extern const char *bmconstidstrings[];

extern char myhostname_BM[];
extern atomic_bool want_garbage_collection_BM;
extern bool gui_is_running_BM;

extern FILE *gui_command_log_file_BM;
////
extern volatile const char*unix_json_socket_BM;

/// these will be realpath-s after initialization
extern const char *contributors_filepath_BM;
extern const char *passwords_filepath_BM;
extern const char *contact_filepath_BM;
extern char *contact_name_BM;
extern char *contact_email_BM;
extern char *password_file_comment_BM;
extern const char *sigusr1_dump_prefix_BM; // at most MAXLEN_SIGUSR1_DUMP_PREFIX_BM bytes

extern const char *myprogname_BM;

extern const char *project_name_BM;
extern const char *plugin_before_load_BM;

extern volatile bool showdebugmsg_BM;
extern bool parsedebugmsg_BM;
extern bool debug_after_load_BM;
extern bool load_bismon_completed_BM;

extern pthread_t mainthreadid_BM;

extern void *dlprog_BM;         // dlopen of entire program
extern struct timespec startrealtimespec_BM;

// the loader, which is non-null only while loading
extern struct loader_stBM *firstloader_BM;
extern char *dump_dir_BM;

extern thread_local struct threadinfo_stBM *curthreadinfo_BM;
extern thread_local volatile struct failurehandler_stBM *curfailurehandle_BM;


extern struct allalloc_stBM *allocationvec_vBM /*¤ allocgc_BM.c */ ;
extern pthread_mutex_t allocationmutex_BM;
#define HAS_PREDEF_BM(Id,Hi,Lo,Hash) \
  extern objectval_tyBM predefdata##Id##_BM; \
  extern objectval_tyBM* predefptr##Id##_BM;
#include "_genbm_predef.h"

#define PREDEF_BM(Id) (predefptr##Id##_BM)

extern const typedhead_tyBM unspecifieddata_BM;
extern int nbworkjobs_BM;

extern volatile struct backstrace_state *backtracestate_BM;


extern char temporary_dir_BM[];

extern bool dont_indent_generated_code_BM;

extern int sigfd_BM;       /* for signalfd(2) */
extern volatile atomic_bool eventlooprunning_BM;








////////////////////////////////////////////////////////////////
// Array describing currently running processes.
extern struct processdescr_stBM running_process_descr_arr_BM[MAXNBWORKJOBS_BM + 1];

/// queued process commands, of nodes (dir, cmd, clos); for processes
/// which are not yet in the array above...
extern struct listtop_stBM *pendingrunproc_list_BM;

// lock for the structures above (both running_process_descr_arr_BM & pendingrunproc_list_BM)
extern pthread_mutex_t pendingrunproc_mtx_BM;


extern pthread_mutex_t onionstack_mtx_BM;
extern pthread_cond_t onionstack_condchange_BM;
extern int cmdpipe_rd_BM, cmdpipe_wr_BM;

#define UNSPECIFIED_BM ((void*)(&unspecifieddata_BM))
#define HAS_GLOBAL_BM(Nam) extern objectval_tyBM*globdata_##Nam##_BM;
#include "_genbm_global.h"
/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "./Build" ;;
 ** End: ;;
 ****************/
#endif /*GLOBALS_BM_INCLUDED */
