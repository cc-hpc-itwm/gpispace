#ifndef SDPA_JOB_HPP
#define SDPA_JOB_HPP 1

#include <string>

namespace sdpa {
    /**
     * The interface to the generic job description we keep around in all
     * components.
     */
    class Job {
    public:
        typedef std::string job_desc_type;
        typedef std::string token_type;
        typedef std::string place_type;
        typedef std::pair<place_desc, token_desc> value_type;
        typedef std::vector<value_type> data_type;

        virtual const job_desc_type & job_description() const = 0;

        virtual const data_type & input_data() const = 0;
        virtual const data_type & output_data() const = 0;

        virtual void add_input(const value_type & value) = 0;
        virtual void add_output(const value_type & value) = 0;
    };
}

#endif
