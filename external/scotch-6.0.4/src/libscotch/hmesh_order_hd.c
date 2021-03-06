/* Copyright 2004,2007 ENSEIRB, INRIA & CNRS
**
** This file is part of the Scotch software package for static mapping,
** graph partitioning and sparse matrix ordering.
**
** This software is governed by the CeCILL-C license under French law
** and abiding by the rules of distribution of free software. You can
** use, modify and/or redistribute the software under the terms of the
** CeCILL-C license as circulated by CEA, CNRS and INRIA at the following
** URL: "http://www.cecill.info".
** 
** As a counterpart to the access to the source code and rights to copy,
** modify and redistribute granted by the license, users are provided
** only with a limited warranty and the software's author, the holder of
** the economic rights, and the successive licensors have only limited
** liability.
** 
** In this respect, the user's attention is drawn to the risks associated
** with loading, using, modifying and/or developing or reproducing the
** software by the user in light of its specific status of free software,
** that may mean that it is complicated to manipulate, and that also
** therefore means that it is reserved for developers and experienced
** professionals having in-depth computer knowledge. Users are therefore
** encouraged to load and test the software's suitability as regards
** their requirements in conditions enabling the security of their
** systems and/or data to be ensured and, more generally, to use and
** operate it in the same conditions as regards security.
** 
** The fact that you are presently reading this means that you have had
** knowledge of the CeCILL-C license and that you accept its terms.
*/
/************************************************************/
/**                                                        **/
/**   NAME       : hmesh_order_hd.c                        **/
/**                                                        **/
/**   AUTHOR     : Francois PELLEGRINI                     **/
/**                                                        **/
/**   FUNCTION   : This module orders a submesh using      **/
/**                the block-oriented Halo Approximate     **/
/**                (Multiple) Minimum Degree algorithm,    **/
/**                with super-variable accounting          **/
/**                R2HAMDf4 v2.0).                         **/
/**                                                        **/
/**   DATES      : # Version 4.0  : from : 10 dec 2003     **/
/**                                 to   : 24 jan 2004     **/
/**                # Version 5.0  : from : 12 sep 2007     **/
/**                                 to   : 12 sep 2007     **/
/**                                                        **/
/************************************************************/

/*
**  The defines and includes.
*/

#define HMESH_ORDER_HD

#include "module.h"
#include "common.h"
#include "graph.h"
#include "order.h"
#include "mesh.h"
#include "hmesh.h"
#include "hall_order_hd.h"
#include "hall_order_hx.h"
#include "hmesh_order_hd.h"
#include "hmesh_order_hx.h"
#include "hmesh_order_si.h"

/*****************************/
/*                           */
/* This is the main routine. */
/*                           */
/*****************************/

/* This routine performs the ordering.
** It returns:
** - 0   : if the ordering could be computed.
** - !0  : on error.
*/

int
hmeshOrderHd (
const Hmesh * RESTRICT const              meshptr,
Order * RESTRICT const                    ordeptr,
const Gnum                                ordenum,
OrderCblk * RESTRICT const                cblkptr, /*+ Single column-block +*/
const HmeshOrderHdParam * RESTRICT const  paraptr)
{
  Gnum * RESTRICT     petab;
  Gnum                pfree;
  Gnum                iwlen;
  Gnum * RESTRICT     iwtab;
  Gnum * RESTRICT     lentab;
  Gnum * RESTRICT     nvartab;
  Gnum * RESTRICT     elentab;
  Gnum * RESTRICT     lasttab;
  Gnum * RESTRICT     leaftab;
  Gnum * RESTRICT     secntab;                    /* Array of index to first secondary variable */
  Gnum * RESTRICT     nexttab;                    /* Array of index of next principal variable  */
  Gnum * RESTRICT     frsttab;
  Gnum                ncmpa;
  Gnum                n;                          /* Number of nodes to order */
  int                 o;

  n = meshptr->m.velmnbr + meshptr->m.vnodnbr;

  if (n < paraptr->colmin)                        /* If graph is too small, order simply */
    return (hmeshOrderSi (meshptr, ordeptr, ordenum, cblkptr));

  iwlen  = (Gnum) ((double) meshptr->m.edgenbr * HMESHORDERHDCOMPRAT) + 32;
  if (iwlen < n)                                  /* Prepare to re-use array */
    iwlen = n;

  if (memAllocGroup ((void **) (void *)
                     &petab,   (size_t) (n     * sizeof (Gnum)),
                     &iwtab,   (size_t) (iwlen * sizeof (Gnum)),
                     &lentab,  (size_t) (n     * sizeof (Gnum)),
                     &nvartab, (size_t) (n     * sizeof (Gnum)),
                     &elentab, (size_t) (n     * sizeof (Gnum)),
                     &lasttab, (size_t) (n     * sizeof (Gnum)),
                     &leaftab, (size_t) (n     * sizeof (Gnum)),
                     &frsttab, (size_t) (n     * sizeof (Gnum)),
                     &secntab, (size_t) (n     * sizeof (Gnum)),
                     &nexttab, (size_t) (n     * sizeof (Gnum)), NULL) == NULL) {
    errorPrint  ("hmeshOrderHd: out of memory");
    return      (1);
  }

  hmeshOrderHxFill (meshptr, petab, lentab, iwtab, elentab, &pfree);

  hallOrderHdHalmd (n, meshptr->m.velmnbr, iwlen, petab, pfree,
                    lentab, iwtab, nvartab, elentab, lasttab, &ncmpa,
                    leaftab, secntab, nexttab, frsttab);

  if (ncmpa < 0) {
    errorPrint ("hmeshOrderHd: internal error");
    memFree    (petab);                           /* Free group leader */
    return     (1);
  }

  o = hallOrderHxBuild (meshptr->m.baseval, n, meshptr->vnohnbr,
                        (meshptr->m.vnumtax == NULL) ? NULL : meshptr->m.vnumtax + (meshptr->m.vnodbas - meshptr->m.baseval), /* Point to node part of vnumtab array */
                        ordeptr, cblkptr,
                        nvartab - meshptr->m.baseval,
                        lentab  - meshptr->m.baseval,
                        petab   - meshptr->m.baseval,
                        frsttab - meshptr->m.baseval,
                        nexttab - meshptr->m.baseval,
                        secntab - meshptr->m.baseval,
                        iwtab   - meshptr->m.baseval,
                        elentab - meshptr->m.baseval,
                        ordeptr->peritab + ordenum, /* Use given inverse permutation as inverse permutation space, never based */
                        leaftab,
                        paraptr->colmin, paraptr->colmax, (float) paraptr->fillrat);

  memFree (petab);                                /* Free group leader */

  return (o);
}
