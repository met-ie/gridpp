#include "Downscaler.h"

Downscaler::Downscaler(Variable::Type iVariable) :
      mVariable(iVariable) {
}

void Downscaler::downscale(const File& iInput, File& iOutput) const {
   downscaleCore(iInput, iOutput);
}

Downscaler* Downscaler::getScheme(std::string iName, Variable::Type iVariable, const Options& iOptions) {
   if(iName == "nearestNeighbour") {
      return new DownscalerNearestNeighbour(iVariable);
   }
   else if(iName == "gradient") {
      DownscalerGradient* d = new DownscalerGradient(iVariable);
      float constantGradient = Util::MV;
      if(iOptions.getValue("constantGradient", constantGradient)) {
         d->setConstantGradient(constantGradient);
      }
      float searchRadius = Util::MV;
      if(iOptions.getValue("searchRadius", searchRadius)) {
         d->setSearchRadius(searchRadius);
      }
      return d;
   }
   else if(iName == "smart") {
      DownscalerSmart* d = new DownscalerSmart(iVariable);
      float searchRadius = Util::MV;
      if(iOptions.getValue("searchRadius", searchRadius)) {
         d->setSearchRadius(searchRadius);
      }
      float numSmart = Util::MV;
      if(iOptions.getValue("numSmart", numSmart)) {
         d->setNumSmart(numSmart);
      }
      return d;
   }
   else {
      Util::error("Could not instantiate downscaler of type '" + iName + "'");
      return NULL;
   }
}