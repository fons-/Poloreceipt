/*
 * "$Id: common.c,v 1.1 2001/08/30 19:37:32 mike Exp $"
 *
 *   Common filter routines for the Common UNIX Printing System (CUPS).
 *
 *   Copyright 1997-2001 by Easy Software Products.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Easy Software Products and are protected by Federal
 *   copyright law.  Distribution and use rights are outlined in the file
 *   "LICENSE.txt" which should have been included with this file.  If this
 *   file is missing or damaged please contact Easy Software Products
 *   at:
 *
 *       Attn: CUPS Licensing Information
 *       Easy Software Products
 *       44141 Airport View Drive, Suite 204
 *       Hollywood, Maryland 20636-3111 USA
 *
 *       Voice: (301) 373-9603
 *       EMail: cups-info@cups.org
 *         WWW: http://www.cups.org
 *
 * Contents:
 *
 *   SetCommonOptions()          - Set common filter options for media size,
 *                                 etc.
 *   UpdatePageVars()            - Update the page variables for the
 *                                 orientation.
 *   WriteClassificationProlog() - Write the prolog with the classification
 *                                 and page label.
 */

/*
 * Include necessary headers...
 */

#include "common.h"


/*
 * Globals...
 */

int	Orientation = 0,	/* 0 = portrait, 1 = landscape, etc. */
	Duplex = 0,		/* Duplexed? */
	LanguageLevel = 1,	/* Language level of printer */
	ColorDevice = 1;	/* Do color text? */
float	PageLeft = 18.0f,	/* Left margin */
	PageRight = 594.0f,	/* Right margin */
	PageBottom = 36.0f,	/* Bottom margin */
	PageTop = 756.0f,	/* Top margin */
	PageWidth = 612.0f,	/* Total page width */
	PageLength = 792.0f;	/* Total page length */


/*
 * 'SetCommonOptions()' - Set common filter options for media size, etc.
 */

ppd_file_t *					/* O - PPD file */
SetCommonOptions(int           num_options,	/* I - Number of options */
                 cups_option_t *options,	/* I - Options */
		 int           change_size)	/* I - Change page size? */
{
  ppd_file_t	*ppd;		/* PPD file */
  ppd_size_t	*pagesize;	/* Current page size */
  const char	*val;		/* Option value */


  ppd = ppdOpenFile(getenv("PPD"));

  ppdMarkDefaults(ppd);
  cupsMarkOptions(ppd, num_options, options);

  if ((pagesize = ppdPageSize(ppd, NULL)) != NULL)
  {
    PageWidth  = pagesize->width;
    PageLength = pagesize->length;
    PageTop    = pagesize->top;
    PageBottom = pagesize->bottom;
    PageLeft   = pagesize->left;
    PageRight  = pagesize->right;

    fprintf(stderr, "DEBUG: Page = %.0fx%.0f; %.0f,%.0f to %.0f,%.0f\n",
            PageWidth, PageLength, PageLeft, PageBottom, PageRight, PageTop);
  }

  if (ppd != NULL)
  {
    ColorDevice   = ppd->color_device;
    LanguageLevel = ppd->language_level;
  }

  if ((val = cupsGetOption("landscape", num_options, options)) != NULL)
    Orientation = 1;

  if ((val = cupsGetOption("orientation-requested", num_options, options)) != NULL)
  {
   /*
    * Map IPP orientation values to 0 to 3:
    *
    *   3 = 0 degrees   = 0
    *   4 = 90 degrees  = 1
    *   5 = -90 degrees = 3
    *   6 = 180 degrees = 2
    */

    Orientation = atoi(val) - 3;
    if (Orientation >= 2)
      Orientation ^= 1;
  }

  if ((val = cupsGetOption("page-left", num_options, options)) != NULL)
  {
    switch (Orientation)
    {
      case 0 :
          PageLeft = (float)atof(val);
	  break;
      case 1 :
          PageBottom = (float)atof(val);
	  break;
      case 2 :
          PageRight = PageWidth - (float)atof(val);
	  break;
      case 3 :
          PageTop = PageLength - (float)atof(val);
	  break;
    }
  }

  if ((val = cupsGetOption("page-right", num_options, options)) != NULL)
  {
    switch (Orientation)
    {
      case 0 :
          PageRight = PageWidth - (float)atof(val);
	  break;
      case 1 :
          PageTop = PageLength - (float)atof(val);
	  break;
      case 2 :
          PageLeft = (float)atof(val);
	  break;
      case 3 :
          PageBottom = (float)atof(val);
	  break;
    }
  }

  if ((val = cupsGetOption("page-bottom", num_options, options)) != NULL)
  {
    switch (Orientation)
    {
      case 0 :
          PageBottom = (float)atof(val);
	  break;
      case 1 :
          PageLeft = (float)atof(val);
	  break;
      case 2 :
          PageTop = PageLength - (float)atof(val);
	  break;
      case 3 :
          PageRight = PageWidth - (float)atof(val);
	  break;
    }
  }

  if ((val = cupsGetOption("page-top", num_options, options)) != NULL)
  {
    switch (Orientation)
    {
      case 0 :
          PageTop = PageLength - (float)atof(val);
	  break;
      case 1 :
          PageRight = PageWidth - (float)atof(val);
	  break;
      case 2 :
          PageBottom = (float)atof(val);
	  break;
      case 3 :
          PageLeft = (float)atof(val);
	  break;
    }
  }

  if (change_size)
    UpdatePageVars();

  if ((val = cupsGetOption("sides", num_options, options)) != NULL &&
      strncasecmp(val, "two-", 4) == 0)
    Duplex = 1;
  else if ((val = cupsGetOption("Duplex", num_options, options)) != NULL &&
           strncasecmp(val, "Duplex", 6) == 0)
    Duplex = 1;
  else if (ppdIsMarked(ppd, "Duplex", "DuplexNoTumble") ||
           ppdIsMarked(ppd, "Duplex", "DuplexTumble"))
    Duplex = 1;

  return (ppd);
}


/*
 * 'UpdatePageVars()' - Update the page variables for the orientation.
 */

void
UpdatePageVars(void)
{
  float		temp;		/* Swapping variable */


  switch (Orientation)
  {
    case 0 : /* Portait */
        break;

    case 1 : /* Landscape */
	temp       = PageLeft;
	PageLeft   = PageBottom;
	PageBottom = temp;

	temp       = PageRight;
	PageRight  = PageTop;
	PageTop    = temp;

	temp       = PageWidth;
	PageWidth  = PageLength;
	PageLength = temp;
	break;

    case 2 : /* Reverse Portrait */
	temp       = PageWidth - PageLeft;
	PageLeft   = PageWidth - PageRight;
	PageRight  = temp;

	temp       = PageLength - PageBottom;
	PageBottom = PageLength - PageTop;
	PageTop    = temp;
        break;

    case 3 : /* Reverse Landscape */
	temp       = PageWidth - PageLeft;
	PageLeft   = PageWidth - PageRight;
	PageRight  = temp;

	temp       = PageLength - PageBottom;
	PageBottom = PageLength - PageTop;
	PageTop    = temp;

	temp       = PageLeft;
	PageLeft   = PageBottom;
	PageBottom = temp;

	temp       = PageRight;
	PageRight  = PageTop;
	PageTop    = temp;

	temp       = PageWidth;
	PageWidth  = PageLength;
	PageLength = temp;
	break;
  }
}


/*
 * 'WriteClassificationProlog()' - Write the prolog with the classification
 *                                 and page label.
 */

void
WriteLabelProlog(const char *label)	/* I - Page label */
{
  const char	*classification;	/* CLASSIFICATION environment variable */


 /*
  * First get the current classification...
  */

  if ((classification = getenv("CLASSIFICATION")) == NULL)
    classification = "";
  if (strcmp(classification, "none") == 0)
    classification = "";

 /*
  * If there is nothing to show, bind an empty 'write labels' procedure
  * and return...
  */

  if (!classification[0] && (label == NULL || !label[0]))
  {
    puts("/espWL{}bind def");
    return;
  }

 /*
  * Set the classification + page label string...
  */

  if (strcmp(classification, "confidential") == 0)
    printf("/espPL(CONFIDENTIAL");
  else if (strcmp(classification, "classified") == 0)
    printf("/espPL(CLASSIFIED");
  else if (strcmp(classification, "secret") == 0)
    printf("/espPL(SECRET");
  else if (strcmp(classification, "topsecret") == 0)
    printf("/espPL(TOP SECRET");
  else if (strcmp(classification, "unclassified") == 0)
    printf("/espPL(UNCLASSIFIED");
  else
    printf("/espPL(");

  if (classification[0] && label)
    printf(" - %s)def\n", label);
  else if (label)
    printf("%s)def\n", label);
  else
    puts(")def");

 /*
  * Then get a 14 point Helvetica-Bold font...
  */

  puts("/espPF /Helvetica-Bold findfont 14 scalefont def");

 /*
  * Finally, the procedure to write the labels on the page...
  */

  puts("/espWL{");
  puts("  espPF setfont");
  printf("  espPL stringwidth pop dup 12 add exch  -0.5 mul %.0f add\n",
         PageWidth * 0.5f);
  puts("  1 setgray");
  printf("  dup 6 sub %.0f 3 index 20 rectfill\n", PageBottom - 2.0);
  printf("  dup 6 sub %.0f 3 index 20 rectfill\n", PageTop - 18.0);
  puts("  0 setgray");
  printf("  dup 6 sub %.0f 3 index 20 rectstroke\n", PageBottom - 2.0);
  printf("  dup 6 sub %.0f 3 index 20 rectstroke\n", PageTop - 18.0);
  printf("  dup %.0f moveto espPL show\n", PageBottom + 2.0);
  printf("  %.0f moveto espPL show\n", PageTop - 14.0);
  puts("pop");
  puts("}bind def");
}


/*
 * End of "$Id: common.c,v 1.1 2001/08/30 19:37:32 mike Exp $".
 */
