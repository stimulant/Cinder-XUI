#include "XAssetManager.h"
#include "cinder/app/App.h"

using namespace xui;

XAsset::XAsset( ci::fs::path relativePath, std::function<void(ci::DataSourceRef)> callback )
:   mRelativePath( relativePath ),
    mCallback( callback ),
    mLastTimeWritten( ci::fs::last_write_time( ci::app::getAssetPath( relativePath ) ) )
{
}

void XAsset::refresh()
{
    std::time_t time = ci::fs::last_write_time( ci::app::getAssetPath( mRelativePath ) );
    if( time > mLastTimeWritten ){
        mLastTimeWritten   = time;
        notify();
    }
}
void XAsset::notify()
{
    mCallback( ci::app::loadAsset( mRelativePath ) );
}



XAssetPair::XAssetPair( ci::fs::path firstRelativePath, ci::fs::path secondRelativePath, std::function<void(ci::DataSourceRef, ci::DataSourceRef)> callback )
:   mRelativePath( std::make_pair( firstRelativePath, secondRelativePath ) ),
    mLastTimeWritten( std::make_pair( ci::fs::last_write_time( ci::app::getAssetPath( firstRelativePath ) ), ci::fs::last_write_time( ci::app::getAssetPath( secondRelativePath ) ) ) ),
    mCallback( callback )
{
}
    
void XAssetPair::refresh()
{
    std::time_t firstTime = ci::fs::last_write_time( ci::app::getAssetPath( mRelativePath.first ) );
    std::time_t secondTime = ci::fs::last_write_time( ci::app::getAssetPath( mRelativePath.second ) );
    if( firstTime > mLastTimeWritten.first || secondTime > mLastTimeWritten.second ){
        mLastTimeWritten.first      = firstTime;
        mLastTimeWritten.second     = secondTime;
        notify();
    }
}
void XAssetPair::notify()
{
    mCallback( ci::app::loadAsset( mRelativePath.first ), ci::app::loadAsset( mRelativePath.second ) );
}


void XAssetManager::load( const ci::fs::path &relativePath, std::function<void(ci::DataSourceRef)> callback )
{
    if( ci::fs::exists( ci::app::getAssetPath( relativePath ) ) ){
        XAssetRef xAsset = XAssetRef( new XAsset( relativePath, callback ) );
        instance()->mAssets.push_back( xAsset );
        xAsset->notify();
    }
    else
        throw ci::app::AssetLoadExc( relativePath );
};
void XAssetManager::load( const ci::fs::path &vertexRelPath, const ci::fs::path &fragmentRelPath, std::function<void(ci::DataSourceRef,ci::DataSourceRef)> callback )
{
    if( ci::fs::exists( ci::app::getAssetPath( vertexRelPath ) ) && ci::fs::exists( ci::app::getAssetPath( fragmentRelPath ) ) ){
        XAssetRef xAsset = XAssetRef( new XAssetPair( vertexRelPath, fragmentRelPath, callback ) );
        instance()->mAssets.push_back( xAsset );
        xAsset->notify();
    }
    else
        throw ci::app::AssetLoadExc( vertexRelPath ); // not necessary correct!
    
}

XAssetManager* XAssetManager::instance()
{
    if( !mInstance ){
        mInstance = new XAssetManager();
        ci::app::App::get()->getSignalUpdate().connect( std::bind( &XAssetManager::update, mInstance ) );
    }
    return mInstance;
}

void XAssetManager::update()
{
    for( std::deque<XAssetRef>::iterator it = mAssets.begin(); it != mAssets.end(); ++it ){
        (*it)->refresh();
    }
}

XAssetManager* XAssetManager::mInstance;