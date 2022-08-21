#ifndef RS0005_FACTORY_H_
#define RS0005_FACTORY_H_

#include "rs_instance_factory.h"

/// @note  This class has been generated from a template. Local changes will not be saved!

namespace tk205 {

   class RS0005Factory : public RSInstanceFactory
   {
      public:
         std::shared_ptr<RSInstanceBase> create_instance(const char* RS_instance_file) const override;
   };
}

#endif
