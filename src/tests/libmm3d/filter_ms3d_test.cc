/*  Maverick Model 3D
 * 
 *  Copyright (c) 2007-2008 Kevin Worcester
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 *  USA.
 *
 *  See the COPYING file for full license text.
 */


// This file tests the MS3D model file filter.

#include <QtTest/QtTest>
#include <unistd.h>

#include "test_common.h"
#include "testfilefactory.h"

#include "model.h"
#include "texture.h"
#include "modelstatus.h"
#include "log.h"
#include "ms3dfilter.h"
#include "mm3dfilter.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"

#include "texmgr.h"
#include "pcxtex.h"
#include "tgatex.h"
#include "rawtex.h"
#include "qttex.h"

int init_std_texture_filters()
{
   log_debug( "initializing standard texture filters\n" );

   TextureManager * textureManager = TextureManager::getInstance();

   TextureFilter * filter;

   filter = new TgaTextureFilter;
   textureManager->registerTextureFilter( filter );
   filter = new RawTextureFilter;
   textureManager->registerTextureFilter( filter );
   filter = new PcxTextureFilter;
   textureManager->registerTextureFilter( filter );
   filter = new QtTextureFilter;
   textureManager->registerTextureFilter( filter );

   return 0;
}


Model * loadModelOrDie( ModelFilter & filter, const char * filename )
{
   Model * model = new Model;
   model->forceAddOrDelete( true );

   Model::ModelErrorE err = filter.readFile( model, filename );

   if ( err != Model::ERROR_NONE )
   {
      fprintf( stderr, "fatal: read %s: %s\n", filename, Model::errorToString( err ) );
      delete model;
      exit( -1 );
   }

   model->loadTextures();
   model->forceAddOrDelete( true );
   model->calculateNormals();
   return model;
}

Model * loadMm3dOrDie( const char * filename, FileFactory * factory = NULL )
{
   MisfitFilter f;
   if ( factory )
      f.setFactory( factory );
   return loadModelOrDie( f, filename );
}

Model * loadMs3dOrDie( const char * filename, FileFactory * factory = NULL )
{
   Ms3dFilter f;
   if ( factory )
      f.setFactory( factory );
   return loadModelOrDie( f, filename );
}

void saveMs3dOrDie( Model * model, const char * filename, FileFactory * factory = NULL )
{
   Ms3dFilter filter;
   if ( factory )
      filter.setFactory( factory );

   Ms3dFilter::Ms3dOptions * opts = new Ms3dFilter::Ms3dOptions;
   opts->setOptionsFromModel( model );

   Model::ModelErrorE err = filter.writeFile( model, filename, opts );

   opts->release();

   if ( err != Model::ERROR_NONE )
   {
      fprintf( stderr, "fatal: write %s: %s\n", filename, Model::errorToString( err ) );
      delete model;
      exit( -1 );
   }
}

//void model_status( Model * model, StatusTypeE type, unsigned ms, const char * fmt, ... )
//{
//   // FIXME hack
//}


class FilterMs3dTest : public QObject
{
   Q_OBJECT
private:
   void testModelFile( const char * lhs_file, const Model * rhs, bool equivOk = false )
   {
      // The lhs pointer is from the original filter
      local_ptr<Model> lhs = loadMm3dOrDie( lhs_file );

      if ( equivOk )
      {
         QVERIFY_TRUE( lhs->equivalent( rhs, 0.001 ) );
      }
      else
      {
         QVERIFY_TRUE( lhs->propEqual( rhs, Model::PartAll, ~(Model::PropInfluences | Model::PropWeights)) );
         QVERIFY_TRUE( lhs->propEqual( rhs, Model::PartVertices, Model::PropInfluences, 0.02 ) );
      }
   }

   void testReadAndWrite( const char * infile, const char * outfile,
         const char * reffile )
   {
      TestFileFactory factory;
      local_ptr<Model> m = loadMs3dOrDie( infile, &factory );
      testModelFile( reffile, m.get() );

      saveMs3dOrDie( m.get(), outfile, &factory );
      m = loadMs3dOrDie( outfile, &factory );
      testModelFile( reffile, m.get() );
   }

private slots:

   void initTestCase()
   {
      init_std_texture_filters();
      log_enable_debug( false );
      log_enable_warning( true );
      log_enable_error( false );
   }

   void testMs3dModelA()
   {
      testReadAndWrite(
            "filtertest/ms3d/zombie02.ms3d",
            "filtertest/ms3d/test_out.ms3d",
            "filtertest/ms3d/zombie02.mm3d" );
   }

   void testMs3dModelB()
   {
      testReadAndWrite(
            "filtertest/ms3d/weighted.ms3d",
            "filtertest/ms3d/test_out.ms3d",
            "filtertest/ms3d/weighted.mm3d" );
   }

   // FIXME add tests:
   //   error handling
   //   options
};

QTEST_MAIN(FilterMs3dTest)
#include "filter_ms3d_test.moc"

