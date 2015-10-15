/****************************************************************************
 *
 * $Id$
 *
 * This file is part of the ViSP software.
 * Copyright (C) 2005 - 2014 by INRIA. All rights reserved.
 * 
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * ("GPL") version 2 as published by the Free Software Foundation.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact INRIA about acquiring a ViSP Professional 
 * Edition License.
 *
 * See http://www.irisa.fr/lagadic/visp/visp.html for more information.
 * 
 * This software was developed at:
 * INRIA Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 * http://www.irisa.fr/lagadic
 *
 * If you have questions regarding the use of this file, please contact
 * INRIA at visp@inria.fr
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 * Description:
 * Tracking of a nurbs.
 *
 * Authors:
 * Nicolas Melchior
 * Fabien Spindler
 *
 *****************************************************************************/

/*!
  \file trackMeNurbs.cpp

  \brief Tracking of a nurbs using vpMe.
*/

/*!
  \example trackMeNurbs.cpp

  Tracking of a nurbs using vpMe.
*/

#include <visp3/core/vpDebug.h>
#include <visp3/core/vpConfig.h>

#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>

#if (defined (VISP_HAVE_X11) || defined(VISP_HAVE_GTK) || defined(VISP_HAVE_GDI) || defined(VISP_HAVE_OPENCV))

#include <visp3/core/vpImage.h>
#include <visp3/core/vpImageIo.h>
#include <visp3/core/vpImagePoint.h>
#include <visp3/core/vpDisplayX.h>
#include <visp3/core/vpDisplayGTK.h>
#include <visp3/core/vpDisplayGDI.h>
#include <visp3/core/vpDisplayOpenCV.h>
#include <visp3/core/vpColor.h>

#include <visp3/core/vpMeNurbs.h>
#include <visp3/core/vpParseArgv.h>
#include <visp3/core/vpIoTools.h>
#include <visp3/core/vpVideoReader.h>

// List of allowed command line options
#define GETOPTARGS	"cdi:h"

void usage(const char *name, const char *badparam, std::string ipath);
bool getOptions(int argc, const char **argv, std::string &ipath, bool &click_allowed, bool &display);

/*!

  Print the program options.

  \param name : Program name.
  \param badparam : Bad parameter name.
  \param ipath : Input image path.

*/
void usage(const char *name, const char *badparam, std::string ipath)
{
  fprintf(stdout, "\n\
Tracking of a nurbs using vpMe.\n\
\n\
SYNOPSIS\n\
  %s [-i <input image path>] [-c] [-d] [-h]\n", name);

  fprintf(stdout, "\n\
OPTIONS:                                               Default\n\
  -i <input image path>                                %s\n\
     Set image input path.\n\
     From this path read images \n\
     \"ViSP-images/ellipse-1/image.%%04d.pgm\"\n\
     Setting the VISP_INPUT_IMAGE_PATH environment\n\
     variable produces the same behaviour than using\n\
     this option.\n\
\n\
  -c\n\
     Disable the mouse click. Useful to automaze the \n\
     execution of this program without humain intervention.\n\
\n\
  -d \n\
     Turn off the display.\n\
\n\
  -h\n\
     Print the help.\n",
	  ipath.c_str());

  if (badparam)
    fprintf(stdout, "\nERROR: Bad parameter [%s]\n", badparam);
}
/*!

  Set the program options.

  \param argc : Command line number of parameters.
  \param argv : Array of command line parameters.
  \param ipath : Input image path.
  \param click_allowed : Mouse click activation.
  \param display : Display activation.

  \return false if the program has to be stopped, true otherwise.

*/
bool getOptions(int argc, const char **argv, std::string &ipath, bool &click_allowed, bool &display)
{
  const char *optarg_;
  int	c;
  while ((c = vpParseArgv::parse(argc, argv, GETOPTARGS, &optarg_)) > 1) {

    switch (c) {
    case 'c': click_allowed = false; break;
    case 'd': display = false; break;
    case 'i': ipath = optarg_; break;
    case 'h': usage(argv[0], NULL, ipath); return false; break;

    default:
      usage(argv[0], optarg_, ipath);
      return false; break;
    }
  }

  if ((c == 1) || (c == -1)) {
    // standalone param or error
    usage(argv[0], NULL, ipath);
    std::cerr << "ERROR: " << std::endl;
    std::cerr << "  Bad argument " << optarg_ << std::endl << std::endl;
    return false;
  }

  return true;
}


int
main(int argc, const char ** argv)
{
  try {
    std::string env_ipath;
    std::string opt_ipath;
    std::string ipath;
    std::string filename;
    bool opt_click_allowed = true;
    bool opt_display = true;

    // Get the visp-images-data package path or VISP_INPUT_IMAGE_PATH environment variable value
    env_ipath = vpIoTools::getViSPImagesDataPath();

    // Set the default input path
    if (! env_ipath.empty())
      ipath = env_ipath;


    // Read the command line options
    if (getOptions(argc, argv, opt_ipath, opt_click_allowed, opt_display) == false) {
      exit (-1);
    }

    // Get the option values
    if (!opt_ipath.empty())
      ipath = opt_ipath;

    // Compare ipath and env_ipath. If they differ, we take into account
    // the input path comming from the command line option
    if (!opt_ipath.empty() && !env_ipath.empty()) {
      if (ipath != env_ipath) {
        std::cout << std::endl
                  << "WARNING: " << std::endl;
        std::cout << "  Since -i <visp image path=" << ipath << "> "
                  << "  is different from VISP_IMAGE_PATH=" << env_ipath << std::endl
                  << "  we skip the environment variable." << std::endl;
      }
    }

    // Test if an input path is set
    if (opt_ipath.empty() && env_ipath.empty()){
      usage(argv[0], NULL, ipath);
      std::cerr << std::endl
                << "ERROR:" << std::endl;
      std::cerr << "  Use -i <visp image path> option or set VISP_INPUT_IMAGE_PATH "
                << std::endl
                << "  environment variable to specify the location of the " << std::endl
                << "  image path where test images are located." << std::endl << std::endl;
      exit(-1);
    }


    // Declare an image, this is a gray level image (unsigned char)
    // it size is not defined yet, it will be defined when the image is
    // read on the disk
    vpImage<unsigned char> I ;

    // Set the path location of the image sequence
    filename = vpIoTools::createFilePath(ipath, "ViSP-images/ellipse-1/image.%04d.pgm");

    // Build the name of the image file
    vpVideoReader reader;
    //Initialize the reader and get the first frame.
    reader.setFileName(filename.c_str());
    reader.setFirstFrameIndex(1);
    reader.open(I);

    // We open a window using either X11, GTK or GDI.
#if defined VISP_HAVE_X11
    vpDisplayX display;
#elif defined VISP_HAVE_GTK
    vpDisplayGTK display;
#elif defined VISP_HAVE_GDI
    vpDisplayGDI display;
#elif defined VISP_HAVE_OPENCV
    vpDisplayOpenCV display;
#endif

    if (opt_display) {
      // Display size is automatically defined by the image (I) size
      display.init(I, 100, 100,"Display...") ;
      // Display the image
      // The image class has a member that specify a pointer toward
      // the display that has been initialized in the display declaration
      // therefore is is no longuer necessary to make a reference to the
      // display variable.
      vpDisplay::display(I) ;
      vpDisplay::flush(I) ;
    }

    vpMeNurbs nurbs ;

    vpMe me ;
    me.setRange(30) ;
    me.setSampleStep(5) ;
    me.setPointsToTrack(60) ;
    me.setThreshold(15000) ;

    nurbs.setMe(&me);
    nurbs.setDisplay(vpMeSite::RANGE_RESULT) ;
    nurbs.setNbControlPoints(14);

    if (opt_click_allowed)
    {
      std::cout << "Click on points along the edge with the left button." << std::endl;
      std::cout << "Then click on the right button to continue." << std::endl;
      nurbs.initTracking(I);
    }
    else
    {
      // Create a list of points to automate the test
      std::list<vpImagePoint> list;
      list.push_back(vpImagePoint(178,357));
      list.push_back(vpImagePoint(212,287));
      list.push_back(vpImagePoint(236,210));
      list.push_back(vpImagePoint(240, 118));
      list.push_back(vpImagePoint(210, 40));

      nurbs.initTracking(I, list) ;
    }
    if (opt_display) {
      nurbs.display(I, vpColor::green) ;
    }

    nurbs.track(I) ;
    if (opt_display && opt_click_allowed) {
      std::cout << "A click to continue..." << std::endl;
      vpDisplay::getClick(I) ;
    }
    std::cout <<"------------------------------------------------------------"<<std::endl;

    for (int iter = 1 ; iter < 40 ; iter++)
    {
      //read the image
      reader.getFrame(I,iter);
      if (opt_display) {
        // Display the image
        vpDisplay::display(I) ;
      }

      //Track the nurbs
      nurbs.track(I) ;


      if (opt_display) {
        nurbs.display(I,vpColor::green) ;
        vpDisplay::flush(I) ;
        vpTime::wait(100);
      }
    }
    if (opt_display && opt_click_allowed) {
      std::cout << "A click to exit..." << std::endl;
      vpDisplay::getClick(I) ;
    }
    return 0;
  }
  catch(vpException e) {
    std::cout << "Catch an exception: " << e.getMessage() << std::endl;
    return 0;
  }
}
#else
int
main()
{
  vpERROR_TRACE("You do not have X11, GTK, GDI or OpenCV display functionalities...");
}

#endif
