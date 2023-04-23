# Find the process IDs for the clk and scheduler processes
clk_pid=$(ps aux | grep clk | awk '{print $2}')
scheduler_pid=$(ps aux | grep scheduler | awk '{print $2}')

# Kill the clk process with SIGINT signal if running
if [ -n "$clk_pid" ]; then
  echo "Terminating clk process with PID $clk_pid..."
  kill -2 "$clk_pid"
fi

# Kill the scheduler process with SIGKILL signal if running
if [ -n "$scheduler_pid" ]; then
  echo "Terminating scheduler process with PID $scheduler_pid..."
