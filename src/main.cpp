#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include "PID.h"
#include <math.h>

// for convenience
using json = nlohmann::json;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

void reset_sim(uWS::WebSocket<uWS::SERVER> ws) {
  std::string msg = "42[\"reset\",{}]";
  ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
}

double sum_vector(const std::vector<double>& s) {
  double rez = 0;
  for (const double& i : s) {
    rez += i;
  }
  return rez;

}
int number = 0;
double best_error = 0.77;
double error = 0;
int n_koef = 0;
bool inc = false;
double prev_speed = 0;
std::vector<double> p{1,0,0};
std::vector<double> dp(3, 1);

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
std::string hasData(std::string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != std::string::npos) {
    return "";
  }
  else if (b1 != std::string::npos && b2 != std::string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}

int main()
{
  uWS::Hub h;

  PID pid;
  // TODO: Initialize the pid variable.
  pid.Init(p);
 
  h.onMessage([&pid](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
  
 
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2')
    { 
      
      auto s = hasData(std::string(data).substr(0, length));
      if (s != "") {
        
        auto j = json::parse(s);
        std::string event = j[0].get<std::string>();
        if (event == "telemetry") {
        
          
          // j[1] is the data JSON object
          double cte = std::stod(j[1]["cte"].get<std::string>());
          //double speed = std::stod(j[1]["speed"].get<std::string>());
          //double angle = std::stod(j[1]["steering_angle"].get<std::string>());
          double steer_value;
          /*
          * TODO: Calcuate steering value here, remember the steering value is
          * [-1, 1].
          * NOTE: Feel free to play around with the throttle and speed. Maybe use
          * another PID controller to control the speed!
          */
          number++;
          error += cte*cte;
          pid.UpdateError(cte);
          steer_value = pid.NewSteer();
          //prev_speed = speed;
          //std::cout << steer_value << std::endl;
          if (fabs(cte) > 3.5) {
            error /= number;
            std::cout << "Best: " << best_error << "    Num: " << error << std::endl;
            std::cout << "p: " << pid.Kp << "    I: " << pid.Ki << "    d: " << pid.Kd << std::endl << std::endl << std::endl;
            
          	if (best_error > error) {
          		best_error = error;
          		number = 0;
          		error = 0;
          		dp[n_koef] *= 1.1;
          		n_koef = (n_koef+1) % 3;
          		p[n_koef] += dp[n_koef];
          		inc = false;
          		pid.Init(p);
          		reset_sim(ws);
          	}
          	else {
          		if (inc == false) {
          			number = 0;
          			error = 0;
          			p[n_koef] -= 2*dp[n_koef];
          			inc = true;
          			pid.Init(p);
          			reset_sim(ws);
          		}
          		else {
          		  p[n_koef] += dp[n_koef];
          			dp[n_koef] *= 0.9;
          			number = 0;
          			error = 0;
          			inc = false;
          			n_koef = (n_koef+1) % 3;
          			p[n_koef] += dp[n_koef];
          			pid.Init(p);
          			reset_sim(ws);
          		}
          	}
          }
          // DEBUG
          //std::cout << number << std::endl;
          //std::cout << "CTE: " << cte << " Steering Value: " << steer_value << std::endl;
          //std::cout << "P_er: " << pid.p_error << "    D_er: " << pid.d_error << "    I_er: "<< pid.i_error << std::endl;
          //std::cout << "p: " << pid.Kp << "    i: " << pid.Ki << "    d: " << pid.Kd <<std::endl;

          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["speed"] = 3;
          msgJson["throttle"] = 0.1;
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          //std::cout << msg << std::endl;
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
          
        }
      } else {
  
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
  
    }
  
  
  
  
  });

  // We don't need this since we're not using HTTP but if it's removed the program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
    const std::string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1)
    {
      res->end(s.data(), s.length());
    }
    else
    {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port))
  {
    std::cout << "Listening to port " << port << std::endl;
  }
  else
  {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  h.run();
}
