/* RMNLIB - Library of useful routines for C and FORTRAN programming
 * Copyright (C) 1975-2001  Division de Recherche en Prevision Numerique
 *                          Environnement Canada
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation,
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "ezscint.h"
#include "ez_funcdef.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
wordint ez_calcxpncof(wordint gdid)
{
  ez_xpncof(&Grille[gdid].i1,&Grille[gdid].i2,&Grille[gdid].j1,&Grille[gdid].j2,&Grille[gdid].extension,
	    Grille[gdid].ni,Grille[gdid].nj,Grille[gdid].grtyp,Grille[gdid].grref,
	    Grille[gdid].ig[IG1], Grille[gdid].ig[IG2], Grille[gdid].ig[IG3], Grille[gdid].ig[IG4],
	    groptions.symmetrie, Grille[gdid].ax, Grille[gdid].ay);
  
   return 0;
   
}
      
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
wordint ez_xpncof(wordint *i1, wordint *i2, wordint *j1, wordint *j2, wordint *extension,
            wordint ni,wordint nj,char grtyp, char grref,
            wordint ig1,wordint ig2,wordint ig3,wordint ig4,wordint sym, ftnfloat *ax, ftnfloat *ay)
{
  
  *i1 = 1;
  *i2 = ni;
  switch (grtyp)
      {
      case '!':
      case 'L':
      case 'N':
      case 'S':
      case 'T':
        *j1 = 1;
        *j2 = nj;
	*extension = 0;
        break;
	
      case 'A':
      case 'G':
	*extension = 2;
        switch (ig1)
	  {
	  case GLOBAL:
	    *j1 = 1;
	    *j2 = nj;
	    break;
	    
	  case NORD:
	    *j1 = -nj+1;
	    *j2 =  nj;
	    break;

	  case SUD:
	    *j1 = 1;
	    *j2 =  2 * nj;
	    break;
	  }
        break;
	
      case 'B':
	*extension = 1;
        switch (ig1)
	  {
	  case GLOBAL:
	    *j1 = 1;
	    *j2 = nj;
	    break;
	    
	  case NORD:
	    *j1 = -nj+2;
	    *j2 = nj;
	    break;


	  case SUD:
	    *j1 = 1;
	    *j2 = 2 * nj - 1;
	    break;
	  }
        break;
	
      case 'E':
        *j1 = 1;
        *j2 = nj;
        break;
	
      case '#':
      case 'Z':
        switch (grref)
	  {
	  case 'E':
	    *j1 = 1;
	    *j2 = nj;
	    if ((ax[ni-1]-ax[0]) < 359.0)
	      {
	      *extension = 0;
	      }
	    else
	      {
	      *extension = 1;
	      }
	    break;
	    
	  default:
	    *j1 = 1;
	    *j2 = nj;
	    *extension = 0;
	    break;
	  }
	break;
	
      default:
	*j1 = 1;
	*j2 = nj;
	*extension = 0;
	break;
      }
  
return 0;
}
