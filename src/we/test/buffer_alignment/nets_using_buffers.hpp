#pragma once

std::string net_with_arbitrary_buffer_sizes_and_alignments
  (unsigned long& total_buffer_size);

std::string net_with_arbitrary_buffer_sizes_and_alignments_insufficient_memory
  (unsigned long& total_buffer_size);

std::string net_with_arbitrary_buffer_sizes_and_default_alignments
  (unsigned long& total_buffer_size);
