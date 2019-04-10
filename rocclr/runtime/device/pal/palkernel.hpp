//
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
//
#pragma once

#include "device/device.hpp"
#include "utils/macros.hpp"
#include "platform/command.hpp"
#include "platform/program.hpp"
#include "platform/kernel.hpp"
#include "platform/sampler.hpp"
#include "device/pal/paldevice.hpp"
#include "device/pal/palvirtual.hpp"
#include "amd_hsa_kernel_code.h"
#include "device/pal/palprintf.hpp"
#include "device/devwavelimiter.hpp"
#include "hsa.h"

namespace amd {
namespace hsa {
namespace loader {
class Symbol;
}  // loader
namespace code {
namespace Kernel {
class Metadata;
}  // Kernel
}  // code
}  // hsa
}  // amd

//! \namespace pal PAL Device Implementation
namespace pal {

class VirtualGPU;
class Device;
class NullDevice;
class HSAILProgram;
class LightningProgram;

/*! \addtogroup pal PAL Device Implementation
 *  @{
 */
class HSAILKernel : public device::Kernel {
 public:

  HSAILKernel(std::string name, HSAILProgram* prog, std::string compileOptions);

  virtual ~HSAILKernel();

  //! Initializes the metadata required for this kernel,
  //! finalizes the kernel if needed
  bool init(amd::hsa::loader::Symbol* sym, bool finalize = false);

  //! Returns GPU device object, associated with this kernel
  const Device& dev() const;

  //! Returns HSA program associated with this kernel
  const HSAILProgram& prog() const;

  //! Returns LDS size used in this kernel
  uint32_t ldsSize() const { return workgroupGroupSegmentByteSize_; }

  //! Returns pointer on CPU to AQL code info
  const amd_kernel_code_t* cpuAqlCode() const { return &akc_; }

  //! Returns memory object with AQL code
  uint64_t gpuAqlCode() const { return code_; }

  //! Returns size of AQL code
  size_t aqlCodeSize() const { return codeSize_; }

  //! Returns the size of argument buffer
  size_t argsBufferSize() const { return kernargSegmentByteSize_; }

  //! Returns spill reg size per workitem
  uint32_t spillSegSize() const { return spillSegmentByteSize_; }

  //! Returns AQL packet in CPU memory
  //! if the kernel arguments were successfully loaded, otherwise NULL
  hsa_kernel_dispatch_packet_t* loadArguments(
      VirtualGPU& gpu,                     //!< Running GPU context
      const amd::Kernel& kernel,           //!< AMD kernel object
      const amd::NDRangeContainer& sizes,  //!< NDrange container
      const_address parameters,            //!< Application arguments for the kernel
      size_t ldsAddress,                   //!< LDS address that includes all arguments.
      uint64_t vmDefQueue,                 //!< GPU VM default queue pointer
      uint64_t* vmParentWrap               //!< GPU VM parent aql wrap object
      ) const;

  //! Returns the kernel index in the program
  uint index() const { return index_; }

 private:
  //! Disable copy constructor
  HSAILKernel(const HSAILKernel&);

  //! Disable operator=
  HSAILKernel& operator=(const HSAILKernel&);

 protected:
  //! Creates AQL kernel HW info
  bool aqlCreateHWInfo(amd::hsa::loader::Symbol* sym);

  //! Get the kernel code and copy the code object from the program CPU segment
  bool setKernelCode(amd::hsa::loader::Symbol* sym, amd_kernel_code_t* akc);

  //! Set up the workgroup info based on the kernel metadata
  void setWorkGroupInfo(const uint32_t privateSegmentSize,
                        const uint32_t groupSegmentSize,
                        const uint16_t numSGPRs,
                        const uint16_t numVGPRs);

  std::string compileOptions_;    //!< compile used for finalizing this kernel
  amd_kernel_code_t akc_;         //!< AQL kernel code on CPU
  uint index_;                    //!< Kernel index in the program

  uint64_t code_;     //!< GPU memory pointer to the kernel
  size_t codeSize_;   //!< Size of ISA code

  uint32_t workgroupGroupSegmentByteSize_;    //!< LDS size used in the kernel
  uint32_t kernargSegmentByteSize_;           //!< Size of kernel argument buffer
  uint32_t spillSegmentByteSize_;             //!< Spill reg size per workitem
};

class LightningKernel : public HSAILKernel {
 public:
  LightningKernel(const std::string& name, HSAILProgram* prog, const std::string& compileOptions)
      : HSAILKernel(name, prog, compileOptions) {}

  //! Returns Lightning program associated with this kernel
  const LightningProgram& prog() const;

  //! Initializes the metadata required for this kernel,
  bool init(amd::hsa::loader::Symbol* symbol);

#if defined(USE_COMGR_LIBRARY)
  //! Initializes the metadata required for this kernel,
  bool init();
#endif
};

/*@}*/} // namespace pal
