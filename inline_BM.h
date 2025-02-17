// file inline_BM.h
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
#ifndef INLINE_BM_INCLUDED
#define INLINE_BM_INCLUDED

const char *
basename_BM (const char *f)
{
  if (!f)
    return "*null*";
  const char *lsh = strrchr (f, '/');
  if (lsh)
    return lsh + 1;
  else
    return f;
}                               /* end basename_BM */

pid_t
gettid_BM (void)
{
  return syscall (SYS_gettid, 0L);
}                               /* end gettid_BM */


double
clocktime_BM (clockid_t clid)
{
  struct timespec ts = { 0, 0 };
  if (clock_gettime (clid, &ts))
    return NAN;
  return (double) ts.tv_sec + 1.0e-9 * ts.tv_nsec;
}

intptr_t
timetoY2Kmillisec_BM (double t)
{
  return (intptr_t) ((t - (double) Y2KEPOCH_BM) * 1000.0);
}                               /* end timetoY2Kmillisec_BM */

double
Y2Kmillisectotime_BM (intptr_t tt)
{
  return (((double) tt * 1.0e-3 + (double) Y2KEPOCH_BM));
}                               /* end Y2Kmillisectotime_BM */

double
cputime_BM (void)
{
  return clocktime_BM (CLOCK_PROCESS_CPUTIME_ID);
}                               /* end cputime_BM */

double
elapsedtime_BM (void)
{
  struct timespec ts = { 0, 0 };
  if (clock_gettime (CLOCK_MONOTONIC, &ts))
    return NAN;
  return (double) (ts.tv_sec - startrealtimespec_BM.tv_sec)
    + 1.0e-9 * (ts.tv_nsec - startrealtimespec_BM.tv_nsec);
}                               /* end elapsedtime_BM */


void
get_realtimespec_delayedms_BM (struct timespec *pts, unsigned millisec)
{
  ASSERT_BM (pts != NULL);
  memset (pts, 0, sizeof (struct timespec));
  clock_gettime (CLOCK_REALTIME, pts);
  pts->tv_nsec += (long) millisec *MILLION_BM;
  while (pts->tv_nsec > BILLION_BM)
    {
      pts->tv_nsec -= (long) BILLION_BM;
      pts->tv_sec++;
    }
}                               /* end get_realtimespec_delayedms_BM */

bool
istaggedint_BM (value_tyBM v)
{
  return ((uintptr_t) v) & 1;
}                               /* end istaggedint_BM */

intptr_t
getint_BM (value_tyBM v)
{
  if (istaggedint_BM (v))
    return ((intptr_t) v) >> 1;
  return 0;
}                               /* end getint_BM */

intptr_t
getintdefault_BM (value_tyBM v, intptr_t def)
{
  if (istaggedint_BM (v))
    return ((intptr_t) v) >> 1;
  return def;
}                               /* end getintdefault_BM */

value_tyBM
taggedint_BM (intptr_t i)
{
  return (value_tyBM) ((i << 1) | 1);
}                               /* end taggedint_BM */

int
valtype_BM (const value_tyBM v)
{
  if (!v)
    return tyNone_BM;
  if (istaggedint_BM (v))
    return tyInt_BM;
  if (((uintptr_t) v & 3) == 0)
    {
      typedhead_tyBM *ht = (typedhead_tyBM *) v;
      WEAKASSERT_BM (ht->htyp != 0);
      return ht->htyp;
    }
  return tyNone_BM;
}                               /* end valtype_BM */


bool
isgenuineval_BM (const value_tyBM v)
{
  int ty = valtype_BM (v);
  if (ty > type_LASTREAL_BM)
    return false;
  return ty != tyNone_BM;
}                               /* end isgenuineval_BM */


objectval_tyBM *
valclass_BM (const value_tyBM v)
{
  if (!v)
    return NULL;
  if (istaggedint_BM (v))
    return BMP_int;
  if (((uintptr_t) v & 3) == 0)
    {
      typedhead_tyBM *ht = (typedhead_tyBM *) v;
      WEAKASSERT_BM (ht->htyp != 0);
      int ty = ht->htyp;
      switch (ty)
        {
        case tyString_BM:
          return BMP_string;
        case tySet_BM:
          return BMP_set;
        case tyTuple_BM:
          return BMP_tuple;
        case tyNode_BM:
          return BMP_node;
        case tyClosure_BM:
          return BMP_closure;
        case tyDouble_BM:
          return BMP_double_float;
        case tyObject_BM:
#ifdef __cplusplus
          {
            objectval_tyBM *ob = ((objectval_tyBM *) v);
            objectval_tyBM *clv = std::atomic_load (&ob->ob_classatomic);
            return clv ? : BMP_undefined;
          }
#else
          {
            objectval_tyBM *ob = ((objectval_tyBM *) v);
            objectval_tyBM *clv = atomic_load (&ob->ob_aclass);
            return clv ? : BMP_undefined;
          }
#endif
        case tyUnspecified_BM:
          return BMP_unspecified;
        }
    }
#ifndef NDEBUG
  weakassertfailureat_BM ("valclass_BM with bad type", __FILE__, __LINE__);
#endif // NDEBUG
  return NULL;
}                               /* end valclass_BM */

hash_tyBM
valhash_BM (const value_tyBM v)
{
  if (!v)
    return 0;
  int ty = valtype_BM (v);
  switch (ty)
    {
    case tyInt_BM:
      {
        uintptr_t i = getint_BM (v);
        hash_tyBM hi = (i & 0x3fffffff) ^ (i % 132594613);
        if (hi == 0)
          hi = ((i % 594571) & 0xfffffff) + 17;
        ASSERT_BM (hi != 0);
        return hi;
      }
    case tyString_BM:
    case tySet_BM:
    case tyTuple_BM:
    case tyNode_BM:
    case tyClosure_BM:
    case tyObject_BM:
      return ((typedhead_tyBM *) v)->hash;
    case tyUnspecified_BM:
      return 8594623;
    default:
      return 0;
    }
}                               /* end valhash_BM */

bool
istuple_BM (const value_tyBM v)
{
  return (valtype_BM (v) == tyTuple_BM);
}                               /* end istuple_BM */

const tupleval_tyBM *
tuplecast_BM (const value_tyBM v)
{
  return istuple_BM (v) ? (const tupleval_tyBM *) v : NULL;
}                               /* end tuplecast_BM */


bool
valequal_BM (const value_tyBM v1, const value_tyBM v2)
{
  if (v1 == v2)
    return true;
  int ty1 = valtype_BM (v1);
  if (ty1 != valtype_BM (v2))
    return false;
  if (valhash_BM (v1) != valhash_BM (v2))
    return false;
  if (ty1 == tyObject_BM)
    return false;
  return valsamecontent_BM (v1, v2, 0);
}                               /* end valequal_BM */

int
valcmp_BM (const value_tyBM v1, const value_tyBM v2)
{
  extern int valcmpdepth_BM (const value_tyBM v1, const value_tyBM v2,
                             int depth);
  if (v1 == v2)
    return 0;
  return valcmpdepth_BM (v1, v2, 0);
}                               /* end valcmp_BM */

void
valarrqsort_BM (value_tyBM * arr, unsigned siz)
{
  if (!arr || siz <= 1)
    return;
  qsort (arr, siz, sizeof (value_tyBM), valqcmp_BM);
}                               /* end valarrqsort_BM */





rawid_tyBM
objid_BM (const objectval_tyBM * obj)
{
  if ((valtype_BM ((value_tyBM) obj) != tyObject_BM))
    return (rawid_tyBM)
    {
    0, 0};
  return obj->ob_id;
}                               /* end objid_BM */

double
objmtime_BM (const objectval_tyBM * obj)
{
  if ((valtype_BM ((value_tyBM) obj) != tyObject_BM))
    return 0.0;
  return obj->ob_mtime;
}                               /* end objmtime_BM */


intptr_t
objmtimeY2Kmilli_BM (const objectval_tyBM * obj)
{
  if ((valtype_BM ((value_tyBM) obj) != tyObject_BM))
    return 0;
  return timetoY2Kmillisec_BM (obj->ob_mtime);
}                               /* end objmtimeY2Kmilli_BM */

void
objtouchmtime_BM (objectval_tyBM * obj, double mtime)
{
  if ((valtype_BM ((value_tyBM) obj) != tyObject_BM))
    return;
  obj->ob_mtime = mtime;
}                               /* end objtouchmtime_BM */


void
objtouchnow_BM (objectval_tyBM * obj)
{
  if ((valtype_BM ((value_tyBM) obj) != tyObject_BM))
    return;
  obj->ob_mtime = clocktime_BM (CLOCK_REALTIME);
}                               /* end objtouchnow_BM */


objectval_tyBM *
objsignature_BM (const objectval_tyBM * obj)
{
  if ((valtype_BM ((value_tyBM) obj) != tyObject_BM))
    return NULL;
  return obj->ob_sig;
}                               /* end objsignature_BM */

void *
objroutaddr_BM (const objectval_tyBM * obj, const objectval_tyBM * objsig)
{
  if ((valtype_BM ((value_tyBM) obj) != tyObject_BM))
    return NULL;
  if (obj->ob_sig != objsig)
    return NULL;
  return obj->ob_routaddr;
}                               /* end objroutaddr_BM */

objectval_tyBM *
objclass_BM (const objectval_tyBM * obj)
{
  if ((valtype_BM ((value_tyBM) obj) != tyObject_BM))
    return NULL;
#ifdef __cplusplus
  {
    objectval_tyBM *clv = std::atomic_load (&obj->ob_classatomic);
    return clv ? : BMP_undefined;
  }
#else
  {
    objectval_tyBM *clv = atomic_load (&obj->ob_aclass);
    return clv ? : BMP_undefined;
  }
#endif
}                               /* end objclass_BM */



/// object support

bool
isobject_BM (const value_tyBM v)
{
  return valtype_BM (v) == tyObject_BM;
}                               /* end isobject_BM */

bool
objlock_BM (objectval_tyBM * obj)
{
  if (valtype_BM (obj) != tyObject_BM)
    return false;
  if (pthread_mutex_lock (&obj->ob_mutex))
    return false;
  if (curfailurehandle_BM)
    {
      ASSERT_BM (curfailurehandle_BM->failh_magic == FAILUREHANDLEMAGIC_BM);
      if (curfailurehandle_BM->failh_lockset)
        register_failock_BM (curfailurehandle_BM->failh_lockset, obj);
    }
  return true;
}                               /* end objlock_BM */

/// this is mostly useful inside asserts, to check that the object was
/// already locked in the same thread.
bool
objislocked_BM (objectval_tyBM * obj)
{
  if (valtype_BM (obj) != tyObject_BM)
    return false;
  int lk = pthread_mutex_trylock (&obj->ob_mutex);
  if (lk == 0)
    {
      pthread_mutex_unlock (&obj->ob_mutex);
      return true;
    }
  return false;
}                               /* end objislocked_BM */

bool
objunlock_BM (objectval_tyBM * obj)
{

  if (valtype_BM (obj) != tyObject_BM)
    return false;
  if (pthread_mutex_unlock (&obj->ob_mutex))
    return false;
  if (curfailurehandle_BM)
    {
      ASSERT_BM (curfailurehandle_BM->failh_magic == FAILUREHANDLEMAGIC_BM);
      if (curfailurehandle_BM->failh_lockset)
        unregister_failock_BM (curfailurehandle_BM->failh_lockset, obj);
    }
  return true;
}                               /* end objunlock_BM */

bool
objtrylock_BM (objectval_tyBM * obj)
{
  if (valtype_BM (obj) != tyObject_BM)
    return false;
  if (pthread_mutex_trylock (&obj->ob_mutex))
    return false;
  if (curfailurehandle_BM)
    {
      ASSERT_BM (curfailurehandle_BM->failh_magic == FAILUREHANDLEMAGIC_BM);
      register_failock_BM (curfailurehandle_BM->failh_lockset, obj);
    };
  return true;
}                               /* end objtrylock_BM */


objectval_tyBM *
objectcast_BM (const value_tyBM v)
{
  return isobject_BM (v) ? (objectval_tyBM *) v : NULL;
}                               /* end objectcast_BM */

hash_tyBM
objecthash_BM (const objectval_tyBM * pob)
{
  if (!isobject_BM ((value_tyBM) pob))
    return 0;
  hash_tyBM h = ((typedhead_tyBM *) pob)->hash;
  ASSERT_BM (h > 0);
  ASSERT_BM (h == hashid_BM (pob->ob_id));
  return h;
}                               /* end objecthash_BM */

unsigned
objspacenum_BM (const objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return 0;
  return obj->ob_space;
}                               /* end objspacenum_BM */

value_tyBM
objgetattr_BM (const objectval_tyBM * obj, const objectval_tyBM * objattr)
{
  if (!isobject_BM ((value_tyBM) obj))
    return NULL;
  if (!isobject_BM ((value_tyBM) objattr))
    return NULL;
  if (!obj->ob_attrassoc)
    return NULL;
  return assoc_getattr_BM (obj->ob_attrassoc, objattr);
}                               /* end objgetattr_BM */

unsigned
objnbattrs_BM (const objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return 0;
  if (!obj->ob_attrassoc)
    return 0;
  return assoc_nbkeys_BM (obj->ob_attrassoc);
}                               /* end objnbattrs_BM */



unsigned
objnbcomps_BM (const objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return 0;
  if (!obj->ob_compvec)
    return 0;
  return datavectlen_BM (obj->ob_compvec);
}                               /* end objnbcomps_BM */

value_tyBM *
objcompdata_BM (const objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return NULL;
  if (!obj->ob_compvec)
    return NULL;
  return (value_tyBM *) datavectdata_BM (obj->ob_compvec);
}                               /* end objcompdata_BM */


const setval_tyBM *
objsetattrs_BM (const objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return NULL;
  if (!obj->ob_attrassoc)
    return NULL;
  return assoc_setattrs_BM (obj->ob_attrassoc);
}                               /* end objsetattrs_BM */


value_tyBM
objgetcomp_BM (const objectval_tyBM * obj, int rk)
{
  if (!isobject_BM ((value_tyBM) obj))
    return NULL;
  if (!obj->ob_compvec)
    return NULL;
  return datavectnth_BM (obj->ob_compvec, rk);
}                               /* end objgetcomp_BM */

value_tyBM
objlastcomp_BM (const objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return NULL;
  if (!obj->ob_compvec)
    return NULL;
  return datavectlast_BM (obj->ob_compvec);
}                               /* end objlastcomp_BM */

void
objputcomp_BM (objectval_tyBM * obj, int rk, const value_tyBM valcomp)
{
  if (!isobject_BM ((value_tyBM) obj))
    return;
  if (!obj->ob_compvec)
    return;
  if (!valcomp || isgenuineval_BM (valcomp))
    datavectputnth_BM (obj->ob_compvec, rk, valcomp);
}                               /* end objputcomp_BM */


void
objpoplastcomp_BM (objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return;
  if (!obj->ob_compvec)
    return;
  obj->ob_compvec = datavect_pop_BM (obj->ob_compvec);
}                               /* end objpoplastcomp_BM */

void
objreservecomps_BM (objectval_tyBM * obj, unsigned gap)
{
  if (!isobject_BM ((value_tyBM) obj))
    return;
  obj->ob_compvec = datavect_reserve_BM (obj->ob_compvec, gap);
}                               /* end objreservecomps_BM */

void
objremoveonecomp_BM (objectval_tyBM * obj, int rk)
{
  if (!isobject_BM ((value_tyBM) obj) || !obj->ob_compvec)
    return;
  obj->ob_compvec = datavect_removeone_BM (obj->ob_compvec, rk);
}                               /* end objremoveonecomp_BM */

void
objinsertonecomp_BM (objectval_tyBM * obj, int rk, const value_tyBM compval)
{
  if (!isobject_BM ((value_tyBM) obj))
    return;
  obj->ob_compvec = datavect_insertone_BM (obj->ob_compvec, rk, compval);
}                               /* end objinsertonecomp_BM */

void
objresetcomps_BM (objectval_tyBM * obj, unsigned len)
{
  if (!isobject_BM ((value_tyBM) obj))
    return;
  obj->ob_compvec = datavect_reserve_BM (NULL, len);
}                               /* end objresetcomps_BM */


void
objappendcomp_BM (objectval_tyBM * obj, value_tyBM compval)
{
  if (!isobject_BM ((value_tyBM) obj))
    return;
  if (compval && !isgenuineval_BM (compval))
    return;
  obj->ob_compvec = datavect_append_BM (obj->ob_compvec, compval);
}                               /* end objappendcomp_BM */


void
objgrowcomps_BM (objectval_tyBM * obj, unsigned gap)
{
  if (!isobject_BM ((value_tyBM) obj))
    return;
  obj->ob_compvec = datavect_grow_BM (obj->ob_compvec, gap);
}                               /* end objgrowcomps_BM */

extendedval_tyBM
objpayload_BM (const objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return NULL;
  return obj->ob_payl;
}                               /* end objpayload_BM */


void
objclearpayload_BM (objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj) || obj == BMP_contributors)
    return;
  extendedval_tyBM payl = obj->ob_payl;
  if (!payl)
    return;
  obj->ob_payl = NULL;
  deleteobjectpayload_BM (obj, payl);
}                               /* end objclearpayload_BM */

void
objputpayload_BM (objectval_tyBM * obj, extendedval_tyBM payl)
{
  if (!isobject_BM ((value_tyBM) obj))
    return;
  extendedval_tyBM oldpayl = obj->ob_payl;
  if (oldpayl == payl)
    return;
  obj->ob_payl = NULL;
  if (oldpayl)
    deleteobjectpayload_BM (obj, oldpayl);
  obj->ob_payl = payl;
}                               /* end objputpayload_BM */

bool
objhasclassinfo_BM (const objectval_tyBM * obj)
{
  extendedval_tyBM payl = objpayload_BM (obj);
  if (!payl)
    return false;
  return (valtype_BM ((value_tyBM) payl) == typayl_classinfo_BM);
}                               /* end objhasclassinfo_BM */

objectval_tyBM *
objgetclassinfosuperclass_BM (const objectval_tyBM * obj)
{
  if (!objhasclassinfo_BM (obj))
    return NULL;
  struct classinfo_stBM *clinf =        //
    (struct classinfo_stBM *) (obj->ob_payl);
  ASSERT_BM (valtype_BM ((value_tyBM) clinf) == typayl_classinfo_BM);
  objectval_tyBM *superob = clinf->clinf_superclass;
  ASSERT_BM (!superob || isobject_BM ((value_tyBM) superob));
  return superob;
}                               /* end objgetclassinfosuperclass_BM */

bool
objectisinstance_BM (const objectval_tyBM * obj,
                     const objectval_tyBM * obclass)
{
  if (!objhasclassinfo_BM (obclass))
    return false;
  if (!isobject_BM ((value_tyBM) obj))
    return false;
  return objclassinfoissubclass_BM (objclass_BM (obj), obclass);
}                               /* end objectisinstance_BM */

const closure_tyBM *
objgetclassinfomethod_BM (const objectval_tyBM * obj,
                          const objectval_tyBM * obselector)
{
  if (!objhasclassinfo_BM (obj))
    return NULL;
  if (!isobject_BM ((value_tyBM) obselector))
    return NULL;
  struct classinfo_stBM *clinf =        //
    (struct classinfo_stBM *) (obj->ob_payl);
  ASSERT_BM (valtype_BM ((value_tyBM) clinf) == typayl_classinfo_BM);
  const closure_tyBM *clos = (const closure_tyBM *)     //
    assoc_getattr_BM (clinf->clinf_dictmeth,
                      obselector);
  ASSERT_BM (!clos || isclosure_BM ((value_tyBM) clos));
  return clos;
}                               /* end objgetclassinfomethod_BM */

const setval_tyBM *             //
objgetclassinfosetofselectors_BM (const objectval_tyBM * obj)
{
  if (!objhasclassinfo_BM (obj))
    return NULL;
  struct classinfo_stBM *clinf =        //
    (struct classinfo_stBM *) (obj->ob_payl);
  ASSERT_BM (valtype_BM ((value_tyBM) clinf) == typayl_classinfo_BM);
  const setval_tyBM *set =      //
    assoc_setattrs_BM (clinf->clinf_dictmeth);
  ASSERT_BM (!set || valtype_BM ((value_tyBM) set) == tySet_BM);
  return set;
}                               /* end objgetclassinfosetofselectors_BM */

////////////////////////////////////////////////////////////////
bool
objhasstrbufferpayl_BM (const objectval_tyBM * obj)
{
  extendedval_tyBM payl = objpayload_BM (obj);
  if (!payl)
    return false;
  int pt = valtype_BM ((value_tyBM) payl);
  return (pt == typayl_strbuffer_BM );
}                               /* end objhasstrbuffer_BM */



struct strbuffer_stBM *
objgetstrbufferpayl_BM (objectval_tyBM * obj)
{
  extendedval_tyBM payl = objpayload_BM (obj);
  if (!payl)
    return NULL;
  int pt = valtype_BM ((value_tyBM) payl);
  if (pt == typayl_strbuffer_BM)
    return (struct strbuffer_stBM *) payl;
  return NULL;
}                               /* end objgetstrbufferpayl_BM */


////////////////
int
objectcmp_BM (const objectval_tyBM * ob1, const objectval_tyBM * ob2)
{
  if (ob1 == ob2)
    return 0;
  if (ob1
      && (((intptr_t) ob1 & 3)
          || ((typedhead_tyBM *) ob1)->htyp != tyObject_BM))
    FATAL_BM ("bad ob1@%p for objectcmp_BM", ob1);
  if (ob2
      && (((intptr_t) ob1 & 3)
          || ((typedhead_tyBM *) ob2)->htyp != tyObject_BM))
    FATAL_BM ("bad ob2@%p for objectcmp_BM", ob2);
  if (!ob1)
    return -1;
  if (!ob2)
    return +1;
  return cmpid_BM (ob1->ob_id, ob2->ob_id);
}                               /* end objectcmp_BM */



int
objectnamedcmp_BM (const objectval_tyBM * ob1, const objectval_tyBM * ob2)
{
  if (ob1 == ob2)
    return 0;
  if (!ob1)
    return -1;
  if (!ob2)
    return +1;
  ASSERT_BM (isobject_BM ((value_tyBM) ob1));
  ASSERT_BM (isobject_BM ((value_tyBM) ob2));
  const char *n1 = findobjectname_BM (ob1);
  const char *n2 = findobjectname_BM (ob2);
  if (n1 && n2)
    return strcmp (n1, n2);
  if (n1)
    return -1;
  if (n2)
    return +1;
  return objectcmp_BM (ob1, ob2);
}                               /* end objectnamedcmp_BM */


////////////////////////////////////////////////////////////////
bool
isassoc_BM (const value_tyBM v)
{
  int ty = valtype_BM (v);
  return ty == typayl_assoctable_BM || ty == typayl_assocpairs_BM;
}                               /* end isassoc_BM */

anyassoc_tyBM *
assoccast_BM (value_tyBM v)
{
  if (isassoc_BM (v))
    return (anyassoc_tyBM *) v;
  return NULL;
}                               /* end assoccast_BM */


anyassoc_tyBM *
make_assoc_BM (unsigned len)
{
  anyassoc_tyBM *res = NULL;
  assoc_reorganize_BM (&res, len);
  return res;
}                               /* end make_assoc_BM */

bool
objhasassocpayl_BM (const objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return false;
  extendedval_tyBM payl = objpayload_BM (obj);
  if (!payl)
    return false;
  return isassoc_BM (payl);
}                               /* end objhasassocpayl_BM */

anyassoc_tyBM *
objgetassocpayl_BM (objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return NULL;
  extendedval_tyBM payl = objpayload_BM (obj);
  if (!payl)
    return NULL;
  return assoccast_BM (payl);
}                               /* end objgetassocpayl_BM */

const setval_tyBM *
objassocsetattrspayl_BM (objectval_tyBM * obj)
{
  anyassoc_tyBM *asso = objgetassocpayl_BM (obj);
  if (!asso)
    return NULL;
  return assoc_setattrs_BM (asso);
}                               /* end objassocsetattrspayl_BM */

unsigned
objassocnbkeyspayl_BM (const objectval_tyBM * obj)
{
  anyassoc_tyBM *asso = objgetassocpayl_BM ((objectval_tyBM *) obj);
  if (!asso)
    return 0;
  return assoc_nbkeys_BM (asso);
}                               /* end objassocnbkeyspayl_BM */

value_tyBM
objassocgetattrpayl_BM (objectval_tyBM * obj, const objectval_tyBM * obattr)
{
  anyassoc_tyBM *asso = objgetassocpayl_BM (obj);
  if (!asso)
    return NULL;
  if (!isobject_BM ((value_tyBM) obattr))
    return NULL;
  return assoc_getattr_BM (asso, obattr);
}                               /* end objassocgetattrpayl_BM */

void
objassocaddattrpayl_BM (objectval_tyBM * obj,
                        const objectval_tyBM * obattr, value_tyBM val)
{
  anyassoc_tyBM *asso = objgetassocpayl_BM (obj);
  if (!asso)
    return;
  if (!isobject_BM ((value_tyBM) obattr))
    return;
  anyassoc_tyBM *newasso = assoc_addattr_BM (asso, obattr, val);
  if (newasso != asso)
    obj->ob_payl = newasso;
}                               /* end objassocaddattrpayl_BM */

void
objassocremoveattrpayl_BM (objectval_tyBM * obj,
                           const objectval_tyBM * obattr)
{
  anyassoc_tyBM *asso = objgetassocpayl_BM (obj);
  if (!asso)
    return;
  if (!isobject_BM ((value_tyBM) obattr))
    return;
  anyassoc_tyBM *newasso = assoc_removeattr_BM (asso, obattr);
  if (newasso != asso)
    objputpayload_BM (obj, newasso);
}                               /* end  objassocremoveattrpayl_BM */

void
objassocreorganizepayl_BM (objectval_tyBM * obj, unsigned gap)
{
  anyassoc_tyBM *asso = objgetassocpayl_BM (obj);
  if (!asso)
    return;
  assoc_reorganize_BM (&asso, gap);
  anyassoc_tyBM *newasso = asso;
  if (newasso != asso)
    objputpayload_BM (obj, newasso);
}                               /* end objassocreorganizepayl_BM */



////////////////
unsigned
datavectlen_BM (const struct datavectval_stBM *dvec)
{
  if (valtype_BM ((value_tyBM) dvec) != typayl_vectval_BM)
    return 0;
  return ((typedsize_tyBM *) dvec)->size;
}                               /* end datavectlen_BM */

const value_tyBM *
datavectdata_BM (const struct datavectval_stBM *dvec)
{
  if (valtype_BM ((value_tyBM) dvec) != typayl_vectval_BM)
    return NULL;
  return dvec->vec_data;
}                               /* end datavectdata_BM */

value_tyBM
datavectnth_BM (const struct datavectval_stBM *dvec, int rk)
{
  unsigned sz = datavectlen_BM (dvec);
  if (rk < 0)
    rk += (int) sz;
  if (rk >= 0 && rk < (int) sz)
    return dvec->vec_data[rk];
  return NULL;
}                               /* end datavectnth_BM */

value_tyBM
datavectlast_BM (const struct datavectval_stBM *dvec)
{
  unsigned sz = datavectlen_BM (dvec);
  if (!sz)
    return NULL;
  return dvec->vec_data[sz - 1];
}                               /* end datavectlast_BM */

void
datavectputnth_BM (struct datavectval_stBM *dvec,
                   int rk, const value_tyBM valcomp)
{
  unsigned sz = datavectlen_BM (dvec);
  if (rk < 0)
    rk += (int) sz;
  if (rk >= 0 && rk < (int) sz)
    dvec->vec_data[rk] = valcomp;
}                               /* end datavectputnth_BM */

struct datavectval_stBM *
datavect_insertone_BM (struct datavectval_stBM *dvec, int rk, value_tyBM val)
{
  return datavect_insert_BM (dvec, rk, &val, 1);
}                               /* end datavect_insertone_BM  */

////////////////

struct datavectval_stBM *
objgetdatavectpayl_BM (objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return NULL;
  void *payl = obj->ob_payl;
  if (!payl)
    return NULL;
  if (valtype_BM (payl) == typayl_vectval_BM)
    return (struct datavectval_stBM *) payl;
  return NULL;
}                               /* end objgetdatavectpayl_BM */

bool
objhasdatavectpayl_BM (objectval_tyBM * obj)
{
  return objgetdatavectpayl_BM (obj) != NULL;
}                               /* end objhasdatavectpayl_BM */


unsigned
objdatavectlengthpayl_BM (objectval_tyBM * obj)
{
  struct datavectval_stBM *dvec = objgetdatavectpayl_BM (obj);
  if (dvec)
    return datavectlen_BM (dvec);
  return 0;
}                               /* end objdatavectlengthpayl_BM */

const value_tyBM *
objdatavectdatapayl_BM (objectval_tyBM * obj)
{
  struct datavectval_stBM *dvec = objgetdatavectpayl_BM (obj);
  if (dvec)
    return datavectdata_BM (dvec);
  return NULL;
}                               /* end objdatavectdatapayl_BM */

void
objdatavectgrowpayl_BM (objectval_tyBM * obj, unsigned gap)
{
  struct datavectval_stBM *dvec = objgetdatavectpayl_BM (obj);
  if (dvec)
    {
      struct datavectval_stBM *newdvec = datavect_grow_BM (dvec, gap);
      if (newdvec != dvec)
        objputpayload_BM (obj, newdvec);
    }
}                               /* end objdatavectgrowpayl_BM */

void
objdatavectreservepayl_BM (objectval_tyBM * obj, unsigned gap)
{
  struct datavectval_stBM *dvec = objgetdatavectpayl_BM (obj);
  if (dvec)

    {
      struct datavectval_stBM *newdvec = datavect_reserve_BM (dvec, gap);
      if (newdvec != dvec)
        objputpayload_BM (obj, newdvec);
    }
}                               /* end objdatavectreservepayl_BM */

void
objdatavectappendpayl_BM (objectval_tyBM * obj, value_tyBM val)
{
  struct datavectval_stBM *dvec = objgetdatavectpayl_BM (obj);
  if (dvec)
    {
      struct datavectval_stBM *newdvec = datavect_append_BM (dvec, val);
      if (newdvec != dvec)
        objputpayload_BM (obj, newdvec);
    }
}                               /* end objdatavectappendpayl_BM */

void
objdatavectinsertpayl_BM (objectval_tyBM * obj,
                          int rk, value_tyBM * valarr, unsigned len)
{
  struct datavectval_stBM *dvec = objgetdatavectpayl_BM (obj);
  if (dvec)
    {
      struct datavectval_stBM *newdvec =
        datavect_insert_BM (dvec, rk, valarr, len);
      if (newdvec != dvec)
        objputpayload_BM (obj, newdvec);
    }
}                               /* end objdatavectinsertpayl_BM */

void
objdatavectinsertonepayl_BM (objectval_tyBM * obj, int rk, value_tyBM val)
{
  struct datavectval_stBM *dvec = objgetdatavectpayl_BM (obj);
  if (dvec)
    {
      struct datavectval_stBM *newdvec =
        datavect_insertone_BM (dvec, rk, val);
      if (newdvec != dvec)
        objputpayload_BM (obj, newdvec);
    }
}                               /* end objdatavectinsertonepayl_BM */

void
objdatavectremovepayl_BM (objectval_tyBM * obj, int rk, unsigned len)
{
  struct datavectval_stBM *dvec = objgetdatavectpayl_BM (obj);
  if (dvec)
    {
      struct datavectval_stBM *newdvec = datavect_remove_BM (dvec, rk, len);
      if (newdvec != dvec)
        objputpayload_BM (obj, newdvec);
    }
}                               /* end objdatavectremovepayl_BM */

const node_tyBM *
objdatavecttonodepayl_BM (objectval_tyBM * obj, const objectval_tyBM * obconn)
{
  if (!isobject_BM ((value_tyBM) obconn))
    return NULL;
  struct datavectval_stBM *dvec = objgetdatavectpayl_BM (obj);
  if (dvec)
    return datavect_to_node_BM (dvec, obconn);
  return NULL;
}                               /* end objdatavecttonodepayl_BM  */


value_tyBM
objdatavectnthpayl_BM (objectval_tyBM * obj, int rk)
{
  struct datavectval_stBM *dvec = objgetdatavectpayl_BM (obj);
  if (dvec)
    return datavectnth_BM (dvec, rk);
  return NULL;
}                               /* end objdatavectnthpayl_BM */

void
objdatavectputnthpayl_BM (objectval_tyBM * obj,
                          int rk, const value_tyBM valcomp)
{
  struct datavectval_stBM *dvec = objgetdatavectpayl_BM (obj);
  if (dvec)
    datavectputnth_BM (dvec, rk, valcomp);
}                               /* end objdatavectputnthpayl_BM */

const tupleval_tyBM *
objdatavecttotuplepayl_BM (objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return NULL;
  struct datavectval_stBM *dvec = objgetdatavectpayl_BM (obj);
  if (!dvec)
    return NULL;
  return datavect_to_tuple_BM (dvec);
}                               /* end objdatavecttotuplepayl_BM */


////////////////////////////////////////////////////////////////

unsigned
decayedvectlen_BM (const struct decayedvectpayl_stBM *dvec)
{
  if (valtype_BM ((value_tyBM) dvec) != typayl_decayed_BM)
    return 0;
  if (dvec->decayp_limitime < elapsedtime_BM ())
    return 0;
  return DECAYEDVECTOR_UCNT_bm (dvec);
}                               /* end decayedvectlen_BM */

unsigned
decayedvectallocsize_BM (const struct decayedvectpayl_stBM *dvec)
{
  if (valtype_BM ((value_tyBM) dvec) != typayl_decayed_BM)
    return 0;
  return DECAYEDVECTOR_ASIZ_bm (dvec);
}                               /* end decayedvectallocsize_BM */


bool
isdecayedvect_BM (const value_tyBM v)
{
  return valtype_BM (v) == typayl_decayed_BM;
}                               /* end isdecayedvect_BM */

const value_tyBM *
decayedvectdata_BM (const struct decayedvectpayl_stBM *dvec)
{
  if (valtype_BM ((value_tyBM) dvec) != typayl_decayed_BM)
    return NULL;
  if (dvec->decayp_limitime < elapsedtime_BM ())
    return NULL;
  return dvec->decayp_arr;
}                               /* end decayedvectdata_BM */

double
decayedvectlimitime_BM (const struct decayedvectpayl_stBM *dvec)
{
  if (valtype_BM ((value_tyBM) dvec) != typayl_decayed_BM)
    return 0;
  return dvec->decayp_limitime;
}                               /* end decayedvectlimitime_BM */


bool
islivedecayedvect_BM (const value_tyBM v)
{
  if (!isdecayedvect_BM (v))
    return false;
  return decayedvectdata_BM ((const struct decayedvectpayl_stBM *) v) != NULL;
}                               /* end islivedecayedvect_BM */

value_tyBM
decayedvectnth_BM (const struct decayedvectpayl_stBM *dvec, int rk)
{
  unsigned sz = decayedvectlen_BM (dvec);
  if (sz == 0)
    return NULL;
  if (rk < 0)
    rk += (int) sz;
  if (rk >= 0 && rk < (int) sz)
    return dvec->decayp_arr[rk];
  return NULL;
}                               /* end decayedvectnth_BM */

value_tyBM
decayedvectlast_BM (const struct decayedvectpayl_stBM *dvec)
{
  unsigned sz = decayedvectlen_BM (dvec);
  if (sz == 0)
    return NULL;
  return dvec->decayp_arr[sz - 1];
}                               /* end decayedvectlast_BM */

bool
decayedvectputnth_BM (struct decayedvectpayl_stBM *dvec,
                      int rk, const value_tyBM valcomp)
{
  unsigned sz = decayedvectlen_BM (dvec);
  if (!sz)
    return false;
  if (rk < 0)
    rk += (int) sz;
  if (rk >= 0 && rk < (int) sz)
    {
      dvec->decayp_arr[rk] = valcomp;
      return true;
    }
  return false;
}                               /* end decayedvectputnth_BM */


bool
decayedvectappend_BM (struct decayedvectpayl_stBM *dvec,
                      const value_tyBM valcomp)
{
  unsigned sz = decayedvectallocsize_BM (dvec);
  if (!sz)
    return false;
  unsigned ln = ((typedsize_tyBM *) dvec)->size;        // DECAYEDVECTOR_UCNT_bm
  if (ln >= sz)
    return false;
  dvec->decayp_arr[ln] = valcomp;
  ((typedsize_tyBM *) dvec)->size++;    // DECAYEDVECTOR_UCNT_bm
  return true;
}                               /* end decayedvectappend_BM  */


struct decayedvectpayl_stBM *
objgetdecayedvectorpayl_BM (objectval_tyBM * obj)
{
  if (!isobject_BM (obj))
    return NULL;
  void *obpayl = objpayload_BM (obj);
  if (isdecayedvect_BM (obpayl))
    return (struct decayedvectpayl_stBM *) obpayl;
  return NULL;
}                               /* end of objgetdecayedvectorpayl_BM */

bool
objhasdecayedvectorpayl_BM (objectval_tyBM * obj)
{
  return objgetdecayedvectorpayl_BM (obj) != NULL;
}                               /* end objhasdecayedvectorpayl_BM */

double
objdecayedvectorlimitimepayl_BM (objectval_tyBM * obj)
{
  struct decayedvectpayl_stBM *dy = objgetdecayedvectorpayl_BM (obj);
  if (!dy)
    return 0.0;
  return decayedvectlimitime_BM (dy);
}                               /* end objdecayedvectorlimitimepayl_BM */


value_tyBM
objdecayedvectornthpayl_BM (objectval_tyBM * obj, int rk)
{
  struct decayedvectpayl_stBM *dy = objgetdecayedvectorpayl_BM (obj);
  if (!dy)
    return NULL;
  return decayedvectnth_BM (dy, rk);
}                               /* end objdecayedvectornthpayl_BM */


value_tyBM
objdecayedvectorlastpayl_BM (objectval_tyBM * obj)
{
  struct decayedvectpayl_stBM *dy = objgetdecayedvectorpayl_BM (obj);
  if (!dy)
    return NULL;
  return decayedvectlast_BM (dy);
}                               /* end objdecayedvectorlastpayl_BM */

const value_tyBM *
objdecayedvectdatapayl_BM (objectval_tyBM * obj)
{
  struct decayedvectpayl_stBM *dy = objgetdecayedvectorpayl_BM (obj);
  if (!dy)
    return NULL;
  return decayedvectdata_BM (dy);
}                               /* end objdecayedvectdatapayl_BM */

unsigned
objdecayedvectlenpayl_BM (objectval_tyBM * obj)
{
  struct decayedvectpayl_stBM *dy = objgetdecayedvectorpayl_BM (obj);
  if (!dy)
    return 0;
  return decayedvectlen_BM (dy);
}                               /* end of objdecayedvectlenpayl_BM   */

unsigned
objdecayedvectallocsizepayl_BM (objectval_tyBM * obj)
{
  struct decayedvectpayl_stBM *dy = objgetdecayedvectorpayl_BM (obj);
  if (!dy)
    return 0;
  return decayedvectallocsize_BM (dy);
}                               /* end of objdecayedvectallocsizepayl_BM   */


bool
objdecayedvectorputnthpayl_BM (objectval_tyBM * obj,
                               int rk, const value_tyBM valcomp)
{
  struct decayedvectpayl_stBM *dy = objgetdecayedvectorpayl_BM (obj);
  if (!dy)
    return false;
  return decayedvectputnth_BM (dy, rk, valcomp);
}                               /* end of objdecayedvectorputnthpayl_BM */


bool
objdecayedvectorappendpayl_BM (objectval_tyBM * obj, const value_tyBM valcomp)
{
  struct decayedvectpayl_stBM *dy = objgetdecayedvectorpayl_BM (obj);
  if (!dy)
    return false;
  return decayedvectappend_BM (dy, valcomp);
}                               /* end of objdecayedvectorappendpayl_BM */


////////////////////////////////////////////////
bool
isset_BM (const value_tyBM v)
{
  return (valtype_BM (v) == tySet_BM);
}                               /* end isset_BM */

const setval_tyBM *
setcast_BM (const value_tyBM v)
{
  return isset_BM (v) ? (const setval_tyBM *) v : NULL;
}                               /* end setcast_BM */

bool
setcontains_BM (const setval_tyBM * setv, const objectval_tyBM * obelem)
{
  if (!isset_BM ((value_tyBM) setv))
    return false;
  if (!isobject_BM ((value_tyBM) obelem))
    return false;
  return setelemindex_BM (setv, obelem) >= 0;
}                               /* end setcontains_BM */

////////////////
bool
issequence_BM (const value_tyBM v)
{
  int ty = valtype_BM (v);
  return (ty == tySet_BM || ty == tyTuple_BM);
}                               /* end issequence_BM */


const seqobval_tyBM *
sequencecast_BM (const value_tyBM v)
{
  return issequence_BM (v) ? (const seqobval_tyBM *) v : NULL;
}                               /* end sequencecast_BM */

bool
isstring_BM (const value_tyBM v)
{
  return (valtype_BM (v) == tyString_BM);
}                               /* end isstring_BM */

const stringval_tyBM *
stringcast_BM (const value_tyBM v)
{
  return isstring_BM (v) ? (const stringval_tyBM *) v : NULL;
}                               /* end stringcast_BM */

bool
hasstring_BM (const value_tyBM v, const char *str)
{
  if (!isstring_BM (v) || !str)
    return false;
  return !strcmp (bytstring_BM ((const stringval_tyBM *) v), str);
}                               /* end hasstring_BM */

////////////////
bool
isdouble_BM (const value_tyBM v)
{
  return (valtype_BM (v) == tyDouble_BM);
}                               /* end isdouble_BM */

const doubleval_tyBM *
doublecast_BM (const value_tyBM val)
{
  if (isdouble_BM (val))
    return (const doubleval_tyBM *) val;
  return NULL;
}                               /* end doublecast_BM */

double
getdouble_BM (const value_tyBM v)
{
  const doubleval_tyBM *db = doublecast_BM (v);
  if (db)
    return db->dbl_num;
  return NAN;
}                               /* end getdouble_BM */

////////////////
struct hashsetobj_stBM *
hashsetobjcast_BM (const value_tyBM v)
{
  if (valtype_BM ((value_tyBM) v) != typayl_hashsetobj_BM)
    return NULL;
  return (struct hashsetobj_stBM *) v;
}                               /* end hashsetobjcast_BM */

unsigned
hashsetobj_cardinal_BM (struct hashsetobj_stBM *hset)
{
  if (valtype_BM ((value_tyBM) hset) != typayl_hashsetobj_BM)
    return 0;
  return ((typedsize_tyBM *) hset)->size;
}                               /* end hashsetobj_cardinal_BM */


struct hashsetobj_stBM *
objgethashsetpayl_BM (objectval_tyBM * obj)
{
  if (!isobject_BM (obj))
    return NULL;
  void *payl = objpayload_BM (obj);
  if (valtype_BM (payl) == typayl_hashsetobj_BM)
    return (struct hashsetobj_stBM *) payl;
  return NULL;
}                               /* end objgethashsetpayl_BM */

bool
objhashashsetpayl_BM (objectval_tyBM * obj)
{
  if (!isobject_BM (obj))
    return false;
  void *payl = objpayload_BM (obj);
  return (valtype_BM (payl) == typayl_hashsetobj_BM);
}                               /* end objhashashsetpayl_BM */

void
objhashsetaddpayl_BM (objectval_tyBM * obj, objectval_tyBM * obelem)
{
  if (!isobject_BM (obelem))
    return;
  struct hashsetobj_stBM *hset = objgethashsetpayl_BM (obj);
  if (hset)
    {
      struct hashsetobj_stBM *newhset = hashsetobj_add_BM (hset, obelem);
      if (newhset != hset)
        objputpayload_BM (obj, newhset);
    }
}                               /* end objhashsetaddpayl_BM */

void
objhashsetremovepayl_BM (objectval_tyBM * obj, objectval_tyBM * obelem)
{
  if (!isobject_BM (obelem))
    return;
  struct hashsetobj_stBM *hset = objgethashsetpayl_BM (obj);
  if (hset)
    {
      struct hashsetobj_stBM *newhset = hashsetobj_remove_BM (hset, obelem);
      if (newhset != hset)
        objputpayload_BM (obj, newhset);
    }
}                               /* end objhashsetremovepayl_BM */

unsigned
objhashsetcardinalpayl_BM (objectval_tyBM * obj)
{
  struct hashsetobj_stBM *hset = objgethashsetpayl_BM (obj);
  if (hset)
    return hashsetobj_cardinal_BM (hset);
  return 0;
}                               /* end objhashsetcardinalpayl_BM */

bool
objhashsetcontainspayl_BM (objectval_tyBM * obj,
                           const objectval_tyBM * obelem)
{
  if (!isobject_BM ((value_tyBM) obelem))
    return false;
  struct hashsetobj_stBM *hset = objgethashsetpayl_BM (obj);
  if (!hset)
    return false;
  return hashsetobj_contains_BM (hset, obelem);
}                               /* end objhashsetcontainspayl_BM */

void
objhashsetgrowpayl_BM (objectval_tyBM * obj, unsigned gap)
{
  struct hashsetobj_stBM *hset = objgethashsetpayl_BM (obj);
  if (!hset)
    return;
  struct hashsetobj_stBM *newhset = hashsetobj_grow_BM (hset, gap);
  if (newhset != hset)
    objputpayload_BM (obj, newhset);
}                               /* end objhashsetgrowpayl_BM */

objectval_tyBM *
objhashsetpickrandompayl_BM (objectval_tyBM * obj)
{
  struct hashsetobj_stBM *hset = objgethashsetpayl_BM (obj);
  if (!hset)
    return NULL;
  return hashsetobj_pick_random_BM (hset);
}                               /* end objhashsetpickrandompayl_BM */

objectval_tyBM *
objhashsettakerandompayl_BM (objectval_tyBM * obj)
{
  struct hashsetobj_stBM *hset = objgethashsetpayl_BM (obj);
  if (!hset)
    return NULL;
  objectval_tyBM *ob = hashsetobj_take_random_BM (hset);
  if (ob)
    {
      unsigned alsiz = ((typedhead_tyBM *) hset)->rlen;
      unsigned ucnt = ((typedsize_tyBM *) hset)->size;
      if (alsiz > 30 && 3 * ucnt < alsiz)
        {
          struct hashsetobj_stBM *newhset = hashsetobj_grow_BM (hset, 0);
          if (newhset != hset)
            objputpayload_BM (obj, newhset);
        }
    }
  return ob;
}                               /* end objhashsettakerandompayl_BM */

const setval_tyBM *
objhashsettosetpayl_BM (objectval_tyBM * obj)
{
  struct hashsetobj_stBM *hset = objgethashsetpayl_BM (obj);
  if (!hset)
    return NULL;
  return hashsetobj_to_set_BM (hset);
}                               /* end objhashsettosetpayl_BM */


////////////////

bool
objhascontributorpayl_BM (const objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return false;
  extendedval_tyBM payl = objpayload_BM (obj);
  if (!payl)
    return false;
  return valtype_BM (payl) == typayl_user_BM;
}                               /* end objhascontributorpayl_BM */

////////////////

bool
islist_BM (const value_tyBM v)
{
  return (valtype_BM (v) == typayl_listtop_BM);
}

value_tyBM
listfirst_BM (const struct listtop_stBM *lis)
{
  if (!islist_BM ((value_tyBM) lis))
    return NULL;
  struct listlink_stBM *firstl = lis->list_first;
  if (!firstl)
    return NULL;
  for (unsigned ix = 0; ix < LINKSIZE_BM; ix++)
    {
      value_tyBM curmem = firstl->link_mems[ix];
      if (curmem)
        return curmem;
    };
  // should never happen
  FATAL_BM ("corrupted list@%p", lis);
}

value_tyBM
listlast_BM (const struct listtop_stBM *lis)
{
  if (!islist_BM ((value_tyBM) lis))
    return NULL;
  struct listlink_stBM *lastl = lis->list_last;
  if (!lastl)
    return NULL;
  for (int ix = LINKSIZE_BM - 1; ix >= 0; ix--)
    {
      value_tyBM curmem = lastl->link_mems[ix];
      if (curmem)
        return curmem;
    };
  // should never happen
  FATAL_BM ("corrupted list@%p", lis);
}

unsigned
listlength_BM (const struct listtop_stBM *lis)
{
  if (!islist_BM ((value_tyBM) lis))
    return 0;
  return ((typedhead_tyBM *) lis)->rlen;
}                               /* end listlength_BM */

////////////////

struct listtop_stBM *
objgetlistpayl_BM (objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return NULL;
  void *payl = objpayload_BM (obj);
  if (valtype_BM ((value_tyBM) payl) == typayl_listtop_BM)
    return (struct listtop_stBM *) payl;
  return NULL;
}                               /* end objgetlistpayl_BM */

bool
objhaslistpayl_BM (objectval_tyBM * obj)
{
  return objgetlistpayl_BM (obj) != NULL;
}                               /* end objhaslistpayl_BM */

static inline value_tyBM
objlistfirstpayl_BM (objectval_tyBM * obj)
{
  struct listtop_stBM *lis = objgetlistpayl_BM (obj);
  if (lis)
    return listfirst_BM (lis);
  return NULL;
}                               /* end objlistfirstpayl_BM */

static inline value_tyBM
objlistlastpayl_BM (objectval_tyBM * obj)
{
  struct listtop_stBM *lis = objgetlistpayl_BM (obj);
  if (lis)
    return listlast_BM (lis);
  return NULL;
}                               /* end objlistlastpayl_BM */

static inline unsigned
objlistlengthpayl_BM (objectval_tyBM * obj)
{
  struct listtop_stBM *lis = objgetlistpayl_BM (obj);
  if (lis)
    return listlength_BM (lis);
  return 0;
}                               /* end objlistlengthpayl_BM */

void
objlistclearpayl_BM (objectval_tyBM * obj)
{
  struct listtop_stBM *lis = objgetlistpayl_BM (obj);
  if (lis)
    listclear_BM (lis);
}                               /* end objlistclearpayl_BM */

void
objlistappendpayl_BM (objectval_tyBM * obj, value_tyBM val)
{
  struct listtop_stBM *lis = objgetlistpayl_BM (obj);
  if (lis)
    listappend_BM (lis, val);
}                               /* end objlistappendpayl_BM */

void
objlistprependpayl_BM (objectval_tyBM * obj, value_tyBM val)
{
  struct listtop_stBM *lis = objgetlistpayl_BM (obj);
  if (lis)
    listprepend_BM (lis, val);
}                               /* end objlistprependpayl_BM */

void
objlistpopfirstpayl_BM (objectval_tyBM * obj)
{
  struct listtop_stBM *lis = objgetlistpayl_BM (obj);
  if (lis)
    listpopfirst_BM (lis);
}                               /* end objlistpopfirstpayl_BM */

void
objlistpoplastpayl_BM (objectval_tyBM * obj)
{
  struct listtop_stBM *lis = objgetlistpayl_BM (obj);
  if (lis)
    listpoplast_BM (lis);
}                               /* end objlistpoplastpayl_BM */

const node_tyBM *
objlisttonodepayl_BM (objectval_tyBM * obj, const objectval_tyBM * obconn)
{
  struct listtop_stBM *lis = objgetlistpayl_BM (obj);
  if (!isobject_BM ((value_tyBM) obconn))
    return NULL;
  if (lis)
    return list_to_node_BM (lis, obconn);
  return NULL;
}                               /* end objlisttonodepayl_BM */

const tupleval_tyBM *
objlisttotuplepayl_BM (objectval_tyBM * obj)
{
  struct listtop_stBM *lis = objgetlistpayl_BM (obj);
  if (lis)
    return list_to_tuple_BM (lis);
  return NULL;
}                               /* end objlisttotuplepayl_BM */

////////////////////////////////////////////////////////////////
bool
istree_BM (const value_tyBM v)
{
  int ty = valtype_BM (v);
  return (ty == tyClosure_BM || ty == tyNode_BM || ty == typayl_quasinode_BM);
}                               /* end istree_BM */

const tree_tyBM *
treecast_BM (const value_tyBM v)
{
  return istree_BM (v) ? (const tree_tyBM *) v : NULL;
}                               /* end treecast_BM */

bool
isclosure_BM (const value_tyBM v)
{
  return valtype_BM (v) == tyClosure_BM;
}                               /* end isclosure_BM */

const closure_tyBM *
closurecast_BM (const value_tyBM v)
{
  return isclosure_BM (v) ? (const closure_tyBM *) v : NULL;
}                               /* end closurecast_BM */

const closure_tyBM *
makeclosure0_BM (const objectval_tyBM * connob)
{
  return makeclosure_BM (connob, 0, NULL);
}                               /* end makeclosure0_BM  */

const closure_tyBM *
makeclosure1_BM (const objectval_tyBM * connob, value_tyBM v0)
{
  value_tyBM arr[1] = { v0 };
  return makeclosure_BM (connob, 1, arr);
}                               /* end makeclosure1_BM  */


const closure_tyBM *
makeclosure2_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1)
{
  value_tyBM arr[2] = { v0, v1 };
  return makeclosure_BM (connob, 2, arr);
}                               /* end makeclosure2_BM  */

const closure_tyBM *
makeclosure3_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1,
                 value_tyBM v2)
{
  value_tyBM arr[3] = { v0, v1, v2 };
  return makeclosure_BM (connob, 3, arr);
}                               /* end makeclosure3_BM  */

const closure_tyBM *
makeclosure4_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1,
                 value_tyBM v2, value_tyBM v3)
{
  value_tyBM arr[4] = { v0, v1, v2, v3 };
  return makeclosure_BM (connob, 4, arr);
}                               /* end makeclosure4_BM  */

const closure_tyBM *
makeclosure5_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1,
                 value_tyBM v2, value_tyBM v3, value_tyBM v4)
{
  value_tyBM arr[5] = { v0, v1, v2, v3, v4 };
  return makeclosure_BM (connob, 5, arr);
}                               /* end makeclosure5_BM  */

const closure_tyBM *
makeclosure6_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1,
                 value_tyBM v2, value_tyBM v3, value_tyBM v4, value_tyBM v5)
{
  value_tyBM arr[6] = { v0, v1, v2, v3, v4, v5 };
  return makeclosure_BM (connob, 6, arr);
}                               /* end makeclosure6_BM  */

const closure_tyBM *
makeclosure7_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1,
                 value_tyBM v2, value_tyBM v3, value_tyBM v4, value_tyBM v5,
                 value_tyBM v6)
{
  value_tyBM arr[7] = { v0, v1, v2, v3, v4, v5, v6 };
  return makeclosure_BM (connob, 7, arr);
}                               /* end makeclosure7_BM  */


const closure_tyBM *
makeclosure8_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1,
                 value_tyBM v2, value_tyBM v3, value_tyBM v4, value_tyBM v5,
                 value_tyBM v6, value_tyBM v7)
{
  value_tyBM arr[8] = { v0, v1, v2, v3, v4, v5, v6, v7 };
  return makeclosure_BM (connob, 8, arr);
}                               /* end makeclosure8_BM  */

////////////////
bool
isnode_BM (const value_tyBM v)
{
  return valtype_BM (v) == tyNode_BM;
}                               /* end isnode_BM */

const node_tyBM *
nodecast_BM (const value_tyBM v)
{
  return isnode_BM (v) ? (const node_tyBM *) v : NULL;
}                               /* end nodecast_BM */


const node_tyBM *
makenode0_BM (const objectval_tyBM * connob)
{
  return makenode_BM (connob, 0, NULL);
}                               /* end makenode0_BM  */

const node_tyBM *
makenode1_BM (const objectval_tyBM * connob, value_tyBM v0)
{
  value_tyBM arr[1] = { v0 };
  return makenode_BM (connob, 1, arr);
}                               /* end makenode1_BM  */


const node_tyBM *
makenode2_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1)
{
  value_tyBM arr[2] = { v0, v1 };
  return makenode_BM (connob, 2, arr);
}                               /* end makenode2_BM  */

const node_tyBM *
makenode3_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1,
              value_tyBM v2)
{
  value_tyBM arr[3] = { v0, v1, v2 };
  return makenode_BM (connob, 3, arr);
}                               /* end makenode3_BM  */

const node_tyBM *
makenode4_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1,
              value_tyBM v2, value_tyBM v3)
{
  value_tyBM arr[4] = { v0, v1, v2, v3 };
  return makenode_BM (connob, 4, arr);
}                               /* end makenode4_BM  */

const node_tyBM *
makenode5_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1,
              value_tyBM v2, value_tyBM v3, value_tyBM v4)
{
  value_tyBM arr[5] = { v0, v1, v2, v3, v4 };
  return makenode_BM (connob, 5, arr);
}                               /* end makenode5_BM  */

const node_tyBM *
makenode6_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1,
              value_tyBM v2, value_tyBM v3, value_tyBM v4, value_tyBM v5)
{
  value_tyBM arr[6] = { v0, v1, v2, v3, v4, v5 };
  return makenode_BM (connob, 6, arr);
}                               /* end makenode6_BM  */

const node_tyBM *
makenode7_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1,
              value_tyBM v2, value_tyBM v3, value_tyBM v4, value_tyBM v5,
              value_tyBM v6)
{
  value_tyBM arr[7] = { v0, v1, v2, v3, v4, v5, v6 };
  return makenode_BM (connob, 7, arr);
}                               /* end makenode7_BM  */


const node_tyBM *
makenode8_BM (const objectval_tyBM * connob, value_tyBM v0, value_tyBM v1,
              value_tyBM v2, value_tyBM v3, value_tyBM v4, value_tyBM v5,
              value_tyBM v6, value_tyBM v7)
{
  value_tyBM arr[8] = { v0, v1, v2, v3, v4, v5, v6, v7 };
  return makenode_BM (connob, 8, arr);
}                               /* end makenode8_BM  */

////////////////

objectval_tyBM *
treeconn_BM (const value_tyBM v)
{
  if (!istree_BM (v))
    return NULL;
  return ((const tree_tyBM *) v)->nodt_conn;
}                               /* end treeconn_BM */

objectval_tyBM *
closureconn_BM (const value_tyBM v)
{
  if (!isclosure_BM (v))
    return NULL;
  return ((const closure_tyBM *) v)->nodt_conn;
}                               /* end closureconn_BM */

objectval_tyBM *
nodeconn_BM (const value_tyBM v)
{
  if (!isnode_BM (v))
    return NULL;
  return ((const node_tyBM *) v)->nodt_conn;
}                               /* end nodeconn_BM */


unsigned
treewidth_BM (const value_tyBM v)
{
  if (!istree_BM (v))
    return 0;
  return ((const typedsize_tyBM *) v)->size;
}                               /* end treewidth_BM */

unsigned
closurewidth_BM (const value_tyBM v)
{
  if (!isclosure_BM (v))
    return 0;
  return ((const typedsize_tyBM *) v)->size;
}                               /* end closurewidth_BM */

unsigned
nodewidth_BM (const value_tyBM v)
{
  if (!isnode_BM (v))
    return 0;
  return ((const typedsize_tyBM *) v)->size;
}

value_tyBM
treenthson_BM (const value_tyBM tr, int rk)
{
  if (!istree_BM (tr))
    return NULL;
  unsigned w = treewidth_BM (tr);
  if (rk < 0)
    rk += (int) w;
  if (rk >= 0 && rk < (int) w)
    return ((const tree_tyBM *) tr)->nodt_sons[rk];
  return NULL;
}                               /* end treenthson_BM */

value_tyBM
closurenthson_BM (const value_tyBM clo, int rk)
{
  if (!isclosure_BM (clo))
    return NULL;
  unsigned w = closurewidth_BM (clo);
  if (rk < 0)
    rk += (int) w;
  if (rk >= 0 && rk < (int) w)
    return ((const closure_tyBM *) clo)->nodt_sons[rk];
  return NULL;
}                               /* end closurenthson_BM */

value_tyBM
nodenthson_BM (const value_tyBM nod, int rk)
{
  if (!isnode_BM (nod))
    return NULL;
  unsigned w = nodewidth_BM (nod);
  if (rk < 0)
    rk += (int) w;
  if (rk >= 0 && rk < (int) w)
    return ((const node_tyBM *) nod)->nodt_sons[rk];
  return NULL;
}                               /* end nodenthson_BM */


bool
isparser_BM (const extendedval_tyBM v)
{
  int ty = valtype_BM (v);
  return ty == typayl_parser_BM;
}                               /* end isparser_BM */

objectval_tyBM *
parserowner_BM (const extendedval_tyBM v)
{
  if (!isparser_BM (v))
    return NULL;
  return ((struct parser_stBM *) v)->pars_ownob;
}                               /* end parserowner_BM */

objectval_tyBM *
checkedparserowner_BM (const extendedval_tyBM v)
{
  objectval_tyBM *obown = parserowner_BM (v);
  if (!obown)
    FATAL_BM ("parser without owner");
  if (objpayload_BM (obown) != v)
    {
      char objidbuf[32];
      memset (objidbuf, 0, sizeof (objidbuf));
      idtocbuf32_BM (objid_BM (obown), objidbuf);
      FATAL_BM ("parser not owned by %s", objidbuf);
    };
  return obown;
}                               /* end checkedparserowner_BM */


struct parser_stBM *
parsercast_BM (const value_tyBM v)
{
  return isparser_BM (v) ? ((struct parser_stBM *) v) : NULL;
};

struct parser_stBM *
objparserpayload_BM (objectval_tyBM * obj)
{
  if (!isobject_BM (obj))
    return NULL;
  struct parser_stBM *pars = parsercast_BM (objpayload_BM (obj));
  if (!pars)
    return NULL;
  ASSERT_BM (checkedparserowner_BM (pars) == obj);
  return pars;
}                               /* end objparserpayload_BM */

unsigned
parserlineno_BM (const struct parser_stBM *pars)
{
  if (isparser_BM ((value_tyBM) pars))
    return pars->pars_lineno;
  return 0;
}                               /* end parserlineno_BM */

unsigned
parsercolpos_BM (const struct parser_stBM *pars)
{
  if (isparser_BM ((value_tyBM) pars))
    return pars->pars_colpos;
  return 0;
}                               /* end parsercolpos_BM */

const char *
parserrestline_BM (const struct parser_stBM *pars)
{
  if (!isparser_BM ((value_tyBM) pars))
    return NULL;
  if (!pars->pars_linebuf)
    return NULL;
  if (pars->pars_linelen < 0)
    return NULL;
  if (pars->pars_curbyte == NULL)
    return NULL;
  ASSERT_BM (pars->pars_curbyte >= pars->pars_linebuf
             && pars->pars_curbyte <=
             pars->pars_linebuf + pars->pars_linesiz);
  return pars->pars_curbyte;
}                               /* end parserrestline_BM */

gunichar
parserunichar_BM (const struct parser_stBM *pars)
{
  if (!isparser_BM ((value_tyBM) pars))
    return 0;
  if (!pars->pars_linebuf)
    return 0;
  ASSERT_BM (pars->pars_curbyte >= pars->pars_linebuf
             && pars->pars_curbyte <=
             pars->pars_linebuf + pars->pars_linesiz);

  return g_utf8_get_char (pars->pars_curbyte);
}                               /* end parserunichar_BM */

bool
parsereol_BM (const struct parser_stBM *pars)
{
  if (!isparser_BM ((value_tyBM) pars))
    return false;
  if (!pars->pars_linebuf)
    return true;
  ASSERT_BM (pars->pars_curbyte >= pars->pars_linebuf
             && pars->pars_curbyte <=
             pars->pars_linebuf + pars->pars_linesiz);

  return pars->pars_curbyte >= pars->pars_linebuf + pars->pars_linesiz;
}                               /* end parsereol_BM */

bool
parserendoffile_BM (const struct parser_stBM *pars)
{
  if (!isparser_BM ((extendedval_tyBM) pars))
    return false;
  if (pars->pars_curbyte == NULL || !pars->pars_file)
    return true;
  return parsereol_BM (pars) && feof (pars->pars_file);
}                               /* end parserendoffile_BM */

bool
parseradvanceutf8_BM (struct parser_stBM *pars, unsigned nbc)
{
  if (!isparser_BM ((extendedval_tyBM) pars))
    return false;
  if (!pars->pars_linebuf)
    return false;
  if (!pars->pars_curbyte)
    return false;
  while (nbc > 0 && !parsereol_BM (pars))
    {
      const char *pc = pars->pars_curbyte;
      if (!*pc)
        return false;
      pc = g_utf8_next_char (pc);
      pars->pars_curbyte = pc;
      pars->pars_colpos++;
      nbc--;
      if (nbc == 0)
        return true;
    }
  return false;
}                               /* end parseradvanceutf8_BM */

////
struct dumper_stBM *
obdumpgetdumper_BM (objectval_tyBM * dumpob)
{
  extendedval_tyBM payl = objpayload_BM (dumpob);
  if (!payl)
    return NULL;
  if (valtype_BM (payl) == typayl_dumper_BM)
    return (struct dumper_stBM *) (payl);
  return NULL;
}                               /* end obdumpgetdumper_BM */

////////////////////////////////////////////////////////////////
bool
isdict_BM (const value_tyBM v)
{
  int ty = valtype_BM (v);
  return ty == typayl_dict_BM;
}                               /* end isdict_BM */

struct dict_stBM *
objgetdictpayl_BM (objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return NULL;
  void *payl = obj->ob_payl;
  if (!payl)
    return NULL;
  if (isdict_BM ((value_tyBM) payl))
    return (struct dict_stBM *) payl;
  return NULL;
}                               /* end objgetdictpayl_BM */

bool
objhasdictpayl_BM (objectval_tyBM * obj)
{
  return objgetdictpayl_BM (obj) != NULL;
}                               /* end objhasdictpayl_BM */

unsigned
objdictsizepayl_BM (objectval_tyBM * obj)
{
  struct dict_stBM *dic = objgetdictpayl_BM (obj);
  if (dic)
    return dictsize_BM (dic);
  return 0;
}                               /* end objdictsizepayl_BM */

value_tyBM
objdictgetpayl_BM (objectval_tyBM * obj, const stringval_tyBM * str)
{
  struct dict_stBM *dic = objgetdictpayl_BM (obj);
  if (dic)
    {
      if (isstring_BM ((value_tyBM) str))
        return dictget_BM (dic, str);
    }
  return NULL;
}                               /* end objdictgetpayl_BM */

void
objdictputpayl_BM (objectval_tyBM * obj,
                   const stringval_tyBM * str, const value_tyBM val)
{
  struct dict_stBM *dic = objgetdictpayl_BM (obj);
  if (dic)
    {
      if (isstring_BM ((value_tyBM) str) && val)
        dictput_BM (dic, str, val);
    }
}                               /* end of objdictputpayl_BM */

void
objdictremovepayl_BM (objectval_tyBM * obj, const stringval_tyBM * str)
{
  struct dict_stBM *dic = objgetdictpayl_BM (obj);
  if (dic)
    {
      if (isstring_BM ((value_tyBM) str))
        dictremove_BM (dic, str);
    }
}                               /* end objdictremovepayl_BM */

const stringval_tyBM *
objdictfirstkeypayl_BM (objectval_tyBM * obj)
{
  struct dict_stBM *dic = objgetdictpayl_BM (obj);
  if (dic)
    return dictfirstkey_BM (dic);
  return NULL;
}                               /* end objdictfirstkeypayl_BM */

const stringval_tyBM *
objdictlastkeypayl_BM (objectval_tyBM * obj)
{
  struct dict_stBM *dic = objgetdictpayl_BM (obj);
  if (dic)
    return dictlastkey_BM (dic);
  return NULL;
}                               /* end objdictlastkeypayl_BM */


const stringval_tyBM *
objdictkeyafterpayl_BM (objectval_tyBM * obj, const stringval_tyBM * str)
{
  struct dict_stBM *dic = objgetdictpayl_BM (obj);
  if (dic)
    {
      if (isstring_BM ((value_tyBM) str))
        return dictkeyafter_BM (dic, str);
    }
  return NULL;
}                               /* end objdictkeyafterpayl_BM */

const stringval_tyBM *
objdictkeysameorafterpayl_BM (objectval_tyBM * obj,
                              const stringval_tyBM * str)
{
  struct dict_stBM *dic = objgetdictpayl_BM (obj);
  if (dic)
    {
      if (isstring_BM ((value_tyBM) str))
        return dictkeysameorafter_BM (dic, str);
    }
  return NULL;
}                               /* end objdictkeysameorafterpayl_BM */

const stringval_tyBM *
objdictkeybeforepayl_BM (objectval_tyBM * obj, const stringval_tyBM * str)
{
  struct dict_stBM *dic = objgetdictpayl_BM (obj);
  if (dic)
    {
      if (isstring_BM ((value_tyBM) str))
        return dictkeybefore_BM (dic, str);
    }
  return NULL;
}                               /* end objdictkeybeforepayl_BM */

const node_tyBM *objdictnodeofkeyspayl_BM
  (objectval_tyBM * obj, const objectval_tyBM * obconn)
{
  struct dict_stBM *dic = objgetdictpayl_BM (obj);
  if (dic && isobject_BM ((value_tyBM) obconn))
    return dictnodeofkeys_BM (dic, obconn);
  return NULL;
}                               /* end objdictnodeofkeyspayl_BM */


////////////////////////////////////////////////////////////////

bool
objhasjansjsonpayl_BM (const objectval_tyBM * obj)
{
  extendedval_tyBM payl = objpayload_BM (obj);
  if (!payl)
    return false;
  return (valtype_BM ((const value_tyBM) payl) == typayl_jansjson_BM);
}                               /* end objhasjansjsonpayl_BM */

json_t *
objgetjansjsonpayl_BM (const objectval_tyBM * obj)
{
  extendedval_tyBM payl = objpayload_BM (obj);
  if (!payl)
    return NULL;
  if (valtype_BM ((const value_tyBM) payl) == typayl_jansjson_BM)
    return ((struct jansjson_stBM *) payl)->jansjson_ptr;
  return NULL;
}                               /* end objgetjansjsonpayl_BM */

////////////////////////////////////////////////////////////////
void
garbage_collect_if_wanted_BM (struct stackframe_stBM *stkf)
{
  if (atomic_load (&want_garbage_collection_BM))
    full_garbage_collection_BM (stkf);
}                               /* end garbage_collect_if_wanted_BM */


////////////////
value_tyBM
sendvar_BM (const value_tyBM recv, const objectval_tyBM * obselector, struct stackframe_stBM *stkf, unsigned nbargs,    // no more than MAXAPPLYARGS_BM-1
            const value_tyBM * argarr)
{
  extern value_tyBM sendtinyvar_BM (const value_tyBM recv,
                                    const objectval_tyBM * obselector,
                                    struct stackframe_stBM *stkf,
                                    unsigned nbargs,
                                    const value_tyBM * argarr);
  extern value_tyBM sendmanyvar_BM (const value_tyBM recv,
                                    const objectval_tyBM * obselector,
                                    struct stackframe_stBM *stkf,
                                    unsigned nbargs,
                                    const value_tyBM * argarr);

  if (!isobject_BM ((value_tyBM) obselector))
    return NULL;
  if (nbargs < TINYSIZE_BM)
    return sendtinyvar_BM (recv, obselector, stkf, nbargs, argarr);
  else if (nbargs < MAXAPPLYARGS_BM)
    return sendmanyvar_BM (recv, obselector, stkf, nbargs, argarr);
  else
    FATAL_BM ("too many arguments %d for sendmanyvar_BM %s",
              nbargs, objectdbg_BM (obselector));
}                               /* end sendvar_BM */


////////////////////////////////////////////////////////////////
bool
ishashsetval_BM (const value_tyBM v)
{
  int ty = valtype_BM (v);
  return ty == typayl_hashsetval_BM;
}                               /* end ishashsetval_BM */


bool
ishashsetvbucket_BM (const value_tyBM v)
{
  int ty = valtype_BM (v);
  return ty == typayl_hashsetvbucket_BM;
}                               /* end ishashsetvbucket_BM */


////////////////////////////////////////////////////////////////
bool
objputhashsetvalpayl_BM (objectval_tyBM * obj, unsigned gap)
{
  if (!isobject_BM ((value_tyBM) obj))
    return false;
  objputpayload_BM (obj, hashsetvalreorganize_BM (NULL, gap + gap / 32 + 1));
  return true;
}                               /* end objputhashsetvalpayl_BM */

struct hashsetval_stBM *
objgethashsetvalpayl_BM (objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return NULL;
  void *payl = obj->ob_payl;
  if (ishashsetval_BM ((value_tyBM) payl))
    return (struct hashsetval_stBM *) payl;
  return NULL;
}                               /* end objgethashsetvalpayl_BM */

bool
objhashashsetvalpayl_BM (objectval_tyBM * obj)
{
  return objgethashsetvalpayl_BM (obj) != NULL;
}                               /* end objhashashsetpayl_BM */

bool
objhashsetvalcontainspayl_BM (objectval_tyBM * obj, value_tyBM val)
{
  struct hashsetval_stBM *hsv = objgethashsetvalpayl_BM (obj);
  if (hsv)
    return hashsetvalcontains_BM (hsv, val);
  return false;
}                               /* end objhashsetvalcontainspayl_BM */

void
objhashsetvalreorganizepayl_BM (objectval_tyBM * obj, unsigned gap)
{
  struct hashsetval_stBM *hsv = objgethashsetvalpayl_BM (obj);
  if (hsv)
    {
      struct hashsetval_stBM *newhsv = hashsetvalreorganize_BM (hsv, gap);
      if (newhsv != hsv)
        objputpayload_BM (obj, newhsv);
    }
}                               /* end objhashsetvalreorganizepayl_BM */

void
objhashsetvalputpayl_BM (objectval_tyBM * obj, value_tyBM val)
{
  struct hashsetval_stBM *hsv = objgethashsetvalpayl_BM (obj);
  if (hsv)
    {
      struct hashsetval_stBM *newhsv = hashsetvalput_BM (hsv, val);
      if (newhsv != hsv)
        objputpayload_BM (obj, newhsv);
    }
}                               /* end objhashsetvalputpayl_BM */

void
objhashsetvalremovepayl_BM (objectval_tyBM * obj, value_tyBM val)
{
  struct hashsetval_stBM *hsv = objgethashsetvalpayl_BM (obj);
  if (hsv)
    {
      struct hashsetval_stBM *newhsv = hashsetvalremove_BM (hsv, val);
      if (newhsv != hsv)
        objputpayload_BM (obj, newhsv);
    }
}                               /* end objhashsetvalputpayl_BM */


value_tyBM
objhashsetvalfirstpayl_BM (objectval_tyBM * obj)
{
  struct hashsetval_stBM *hsv = objgethashsetvalpayl_BM (obj);
  if (hsv)
    return hashsetvalfirst_BM (hsv);
  return NULL;
}                               /* end objhashsetvalfirstpayl_BM */

value_tyBM
objhashsetvalnextpayl_BM (objectval_tyBM * obj, value_tyBM prev)
{
  struct hashsetval_stBM *hsv = objgethashsetvalpayl_BM (obj);
  if (hsv)
    return hashsetvalnext_BM (hsv, prev);
  return NULL;
}                               /* end objhashsetvalnextpayl_BM */


value_tyBM objhashsetvalmakenodepayl_BM
  (objectval_tyBM * obj, objectval_tyBM * connob)
{
  struct hashsetval_stBM *hsv = objgethashsetvalpayl_BM (obj);
  if (hsv)
    if (isobject_BM ((value_tyBM) connob))
      return hashsetvalmakenode_BM (hsv, connob);
  return NULL;
}                               /* end objhashsetvalmakenodepayl_BM */

////////////////////////////////////////////////////////////////

bool
ishashmapval_BM (const value_tyBM v)
{
  int ty = valtype_BM (v);
  return ty == typayl_hashmapval_BM;
}                               /* end ishashmapval_BM */

unsigned
hashmapvalsize_BM (const value_tyBM v)
{
  if (!ishashsetval_BM (v))
    return 0;
  return ((typedsize_tyBM *) v)->size;
}                               /* end hashmapvalsize_BM */

bool
ishashmapbucket_BM (const value_tyBM v)
{
  int ty = valtype_BM (v);
  return ty == typayl_hashmapbucket_BM;
}                               /* end ishashmapbucket_BM */

struct hashmapval_stBM *
objgethashmapvalpayl_BM (objectval_tyBM * obj)
{
  if (!isobject_BM ((value_tyBM) obj))
    return NULL;
  void *payl = objpayload_BM (obj);
  if (ishashmapval_BM ((value_tyBM) payl))
    return (struct hashmapval_stBM *) payl;
  return NULL;
}                               /* end objgethashmapvalpayl_BM */

bool
objhashashmapvalpayl_BM (objectval_tyBM * obj)
{
  return objgethashmapvalpayl_BM (obj) != NULL;
}                               /* end objhashashmapvalpayl_BM */

unsigned
objhashmapvalsizepayl_BM (objectval_tyBM * obj)
{
  struct hashmapval_stBM *hma = objgethashmapvalpayl_BM (obj);
  if (hma)
    return hashmapvalsize_BM (hma);
  return 0;
}                               /* end objhashmapvalsizepayl_BM */

value_tyBM
objhashmapvalgetpayl_BM (objectval_tyBM * obj, value_tyBM keyv)
{
  struct hashmapval_stBM *hma = objgethashmapvalpayl_BM (obj);
  if (hma)
    return hashmapvalget_BM (hma, keyv);
  return NULL;
}                               /* end objhashmapvalgetpayl_BM */

void
objhashmapvalreorganizepayl_BM (objectval_tyBM * obj, unsigned gap)
{
  struct hashmapval_stBM *hma = objgethashmapvalpayl_BM (obj);
  if (hma)
    {
      struct hashmapval_stBM *newhma = hashmapvalreorganize_BM (hma, gap);
      if (newhma != hma)
        objputpayload_BM (obj, newhma);
    }
}                               /* end objhashmapvalreorganizepayl_BM */

void
objhashmapvalputpayl_BM (objectval_tyBM * obj, value_tyBM keyv,
                         value_tyBM valv)
{
  struct hashmapval_stBM *hma = objgethashmapvalpayl_BM (obj);
  if (hma)
    {
      struct hashmapval_stBM *newhma = hashmapvalput_BM (hma, keyv, valv);
      if (newhma != hma)
        objputpayload_BM (obj, newhma);
    }
}                               /* end objhashmapvalputpayl_BM */


void
objhashmapvalremovepayl_BM (objectval_tyBM * obj, value_tyBM keyv)
{
  struct hashmapval_stBM *hma = objgethashmapvalpayl_BM (obj);
  if (hma)
    {
      struct hashmapval_stBM *newhma = hashmapvalremove_BM (hma, keyv);
      if (newhma != hma)
        obj->ob_payl = newhma;
    }
}                               /* end objhashmapvalremovepayl_BM */


value_tyBM
objhashmapvalfirstkeypayl_BM (objectval_tyBM * obj)
{
  struct hashmapval_stBM *hma = objgethashmapvalpayl_BM (obj);
  if (hma)
    return hashmapvalfirstkey_BM (hma);
  return NULL;
}                               /* end objhashmapvalremovepayl_BM */

value_tyBM
objhashmapvalnextkeypayl_BM (objectval_tyBM * obj, value_tyBM prevk)
{
  struct hashmapval_stBM *hma = objgethashmapvalpayl_BM (obj);
  if (hma)
    return hashmapvalnextkey_BM (hma, prevk);
  return NULL;
}                               /* end objhashmapvalnextkeypayl_BM  */

value_tyBM
objhashmapvalmakenodeofkeyspayl_BM (objectval_tyBM * obj,
                                    objectval_tyBM * connob)
{
  struct hashmapval_stBM *hma = objgethashmapvalpayl_BM (obj);
  if (hma)
    return hashmapvalmakenodeofkeys_BM (hma, connob);
  return NULL;
}                               /* end objhashmapvalmakenodeofkeyspayl_BM  */



////////////////////////////////////////////////////////////////
#endif /*INLINE_BM_INCLUDED */
