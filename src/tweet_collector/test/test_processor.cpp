#include "../include/tweetoscopeCollectorParams.hpp"
#include "../include/tweetoscopeConsumer.hpp"
#include "../include/tweetoscopeProducer.hpp"
#include "../include/tweetoscopeCascade.hpp"
#include "../include/tweetoscopeTweetReader.hpp"
#include "../include/tweetoscopeProcessor.hpp"

#include <string>
#include<memory>

int main(int argc, char* argv[]) {

  if(argc != 2) {
    std::cout << "Usage : " << argv[0] << " <config-filename>" << std::endl;
    return 0;
  }
  tweetoscope::params::collector params(argv[1]);

  tweetoscope::Consumer consumer(params);
  consumer.subscribe_topic();
  
  // auto producer = std::make_shared<tweetoscope::Producer>(params, params.topic.out_series);

  tweetoscope::ref_producer producer1 = tweetoscope::make_producer(params, params.topic.out_series);
  tweetoscope::ref_producer producer2 = tweetoscope::make_producer(params, params.topic.out_properties);
  tweetoscope::ref_producer producer3 = tweetoscope::make_producer(params, params.topic.out_logs);

  std::cout << std::endl
        << "Parameters : " << std::endl
        << "----------"    << std::endl
        << std::endl
        << params << std::endl
        << std::endl;
        
  unsigned int i = 0;

  tweetoscope::timestamp max_duration = params.times.terminated;
  std::vector<tweetoscope::timestamp> observation_windows = params.times.observation;

  std::map<tweetoscope::source::idf, tweetoscope::ref_processor> processors;

  while(true){
    auto msg = consumer.poll();

    if( msg && ! msg.get_error() ) {

      tweetoscope::tweet twt;
      auto key = tweetoscope::cascade::idf(std::stoi(msg.get_key()));
      auto istr = std::istringstream(std::string(msg.get_payload()));
      istr >> twt;

      if(processors.find(key) == processors.end()){
        processors[key] = tweetoscope::make_processor(
          producer1,
          producer2,
          producer3,
          max_duration,
          params.cascade.min_cascade_size,
          observation_windows);
      }

      processors[key]->process(twt);

    //   tweetoscope::ref_cascade ptr_cascade = tweetoscope::make_cascade(key, twt);
    //   std::cout << *ptr_cascade << std::endl;
    //   i += 1;
      // producer.post_msg(std::to_string(key), std::string(msg.get_payload()));
      // std::cout << key << ": ";
      // std::cout << std::string(msg.get_payload()) << std::endl;
    }
  }
  return 0;
}