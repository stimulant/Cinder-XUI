#pragma once

#include "cinder/Filesystem.h"
#include "cinder/DataSource.h"

namespace xui {

typedef std::shared_ptr<class XAsset> XAssetRef;

class XAsset {
public:
    XAsset(){}
    XAsset( ci::fs::path relativePath, std::function<void(ci::DataSourceRef)> callback );
    
    virtual void refresh();
    virtual void notify();
    
protected:
    ci::fs::path                            mRelativePath;
    std::time_t                             mLastTimeWritten;
    std::function<void(ci::DataSourceRef)>  mCallback;
};

class XAssetPair : public XAsset {
public:
    XAssetPair( ci::fs::path firstRelativePath, ci::fs::path secondRelativePath, std::function<void(ci::DataSourceRef, ci::DataSourceRef)> callback );
    
    virtual void refresh();
    virtual void notify();
    
protected:
    std::pair<ci::fs::path,ci::fs::path>                        mRelativePath;
    std::pair<std::time_t,std::time_t>                          mLastTimeWritten;
    std::function<void(ci::DataSourceRef, ci::DataSourceRef)>   mCallback;
};

class XAssetManager {
public:
    
    static void load( const ci::fs::path &relativePath, std::function<void(ci::DataSourceRef)> callback );
    static void load( const ci::fs::path &vertexRelPath, const ci::fs::path &fragmentRelPath, std::function<void(ci::DataSourceRef,ci::DataSourceRef)> callback );
    
protected:
    XAssetManager(){}
    
    static XAssetManager* instance();
    
    void update();
    
    std::deque< XAssetRef > mAssets;
    static XAssetManager* mInstance;
};

}