import json
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import re
from pathlib import Path
import seaborn as sns

class BenchmarkPlotter:
    def __init__(self, json_file=None):
        """
        Initialize the benchmark plotter.
        
        Args:
            json_file: Path to Google Benchmark JSON output file
        """
        self.data = None
        if json_file:
            self.load_data(json_file)
    
    def load_data(self, json_file):
        """Load benchmark data from JSON file."""
        with open(json_file, 'r') as f:
            self.raw_data = json.load(f)
        
        # Extract benchmark results
        benchmarks = []
        for bench in self.raw_data['benchmarks']:
            # Skip aggregate results (mean, median, stddev)
            if bench.get('aggregate_name'):
                continue
                
            benchmarks.append({
                'name': bench['name'],
                'real_time': bench['real_time'],
                'cpu_time': bench['cpu_time'],
                'time_unit': bench['time_unit'],
                'iterations': bench['iterations'],
                'threads': bench.get('threads', 1),
                'bytes_per_second': bench.get('bytes_per_second', 0),
                'items_per_second': bench.get('items_per_second', 0)
            })
        
        self.data = pd.DataFrame(benchmarks)
        self._parse_benchmark_names()
    
    def _parse_benchmark_names(self):
        """Parse benchmark names to extract implementation and thread count."""
        parsed_data = []
        
        for _, row in self.data.iterrows():
            name = row['name']
            
            # Extract implementation name (everything before the first '/')
            impl_match = re.match(r'^([^/]+)', name)
            implementation = impl_match.group(1) if impl_match else 'Unknown'
            
            # Extract thread count from name or use threads column
            thread_match = re.search(r'/threads:(\d+)', name)
            if thread_match:
                thread_count = int(thread_match.group(1))
            else:
                # Try to extract from patterns like BM_QueueName_8 (8 threads)
                thread_match = re.search(r'_(\d+)$', name)
                thread_count = int(thread_match.group(1)) if thread_match else row['threads']
            
            # Extract operation type if present
            operation = 'mixed'
            if 'enqueue' in name.lower():
                operation = 'enqueue'
            elif 'dequeue' in name.lower():
                operation = 'dequeue'
            elif 'producer' in name.lower():
                operation = 'producer'
            elif 'consumer' in name.lower():
                operation = 'consumer'
            
            parsed_data.append({
                'implementation': implementation,
                'threads': thread_count,
                'operation': operation,
                'throughput_ops_sec': 1e9 / row['real_time'] if row['real_time'] > 0 else 0,
                'latency_ns': row['real_time'],
                'cpu_time_ns': row['cpu_time'],
                'original_name': name
            })
        
        self.parsed_data = pd.DataFrame(parsed_data)
    
    def plot_scalability(self, metric='throughput_ops_sec', figsize=(12, 8), 
                        operation_filter=None, log_scale=False):
        """
        Plot scalability comparison across implementations.
        
        Args:
            metric: 'throughput_ops_sec' or 'latency_ns'
            figsize: Figure size tuple
            operation_filter: Filter by operation type ('enqueue', 'dequeue', 'mixed', etc.)
            log_scale: Use logarithmic scale for y-axis
        """
        if self.parsed_data is None:
            raise ValueError("No data loaded. Call load_data() first.")
        
        data = self.parsed_data.copy()
        
        if operation_filter:
            data = data[data['operation'] == operation_filter]
        
        plt.figure(figsize=figsize)
        
        # Group by implementation and plot
        for impl in data['implementation'].unique():
            impl_data = data[data['implementation'] == impl].sort_values('threads')
            
            plt.plot(impl_data['threads'], impl_data[metric], 
                    marker='o', linewidth=2, markersize=6, label=impl)
        
        plt.xlabel('Number of Threads')
        
        if metric == 'throughput_ops_sec':
            plt.ylabel('Throughput (Operations/Second)')
            plt.title(f'Queue Implementation Scalability - Throughput')
        else:
            plt.ylabel('Latency (nanoseconds)')
            plt.title(f'Queue Implementation Scalability - Latency')
        
        if log_scale:
            plt.yscale('log')
            plt.xscale('log')
        
        plt.grid(True, alpha=0.3)
        plt.legend()
        plt.tight_layout()
        return plt.gcf()
    
    def plot_heatmap(self, metric='throughput_ops_sec', figsize=(10, 6)):
        """Create a heatmap showing performance across implementations and thread counts."""
        if self.parsed_data is None:
            raise ValueError("No data loaded. Call load_data() first.")
        
        # Create pivot table
        pivot_data = self.parsed_data.pivot_table(
            values=metric, 
            index='implementation', 
            columns='threads',
            aggfunc='mean'
        )
        
        plt.figure(figsize=figsize)
        
        if metric == 'throughput_ops_sec':
            sns.heatmap(pivot_data, annot=True, fmt='.2e', cmap='YlOrRd')
            plt.title('Throughput Heatmap (Operations/Second)')
        else:
            sns.heatmap(pivot_data, annot=True, fmt='.0f', cmap='YlOrRd_r')
            plt.title('Latency Heatmap (nanoseconds)')
        
        plt.xlabel('Number of Threads')
        plt.ylabel('Implementation')
        plt.tight_layout()
        return plt.gcf()
    
    def plot_efficiency(self, figsize=(12, 8)):
        """Plot parallel efficiency (speedup / thread_count)."""
        if self.parsed_data is None:
            raise ValueError("No data loaded. Call load_data() first.")
        
        plt.figure(figsize=figsize)
        
        for impl in self.parsed_data['implementation'].unique():
            impl_data = self.parsed_data[self.parsed_data['implementation'] == impl].sort_values('threads')
            
            # Calculate speedup relative to single thread performance
            single_thread_perf = impl_data[impl_data['threads'] == 1]['throughput_ops_sec'].iloc[0] if len(impl_data[impl_data['threads'] == 1]) > 0 else impl_data['throughput_ops_sec'].min()
            
            speedup = impl_data['throughput_ops_sec'] / single_thread_perf
            efficiency = speedup / impl_data['threads']
            
            plt.plot(impl_data['threads'], efficiency, 
                    marker='o', linewidth=2, markersize=6, label=impl)
        
        # Add ideal efficiency line
        max_threads = self.parsed_data['threads'].max()
        plt.axhline(y=1.0, color='black', linestyle='--', alpha=0.5, label='Ideal Efficiency')
        
        plt.xlabel('Number of Threads')
        plt.ylabel('Parallel Efficiency')
        plt.title('Parallel Efficiency Comparison')
        plt.grid(True, alpha=0.3)
        plt.legend()
        plt.ylim(0, None)
        plt.tight_layout()
        return plt.gcf()
    
    def generate_report(self, output_dir='.', save_plots=True):
        """Generate a comprehensive performance report."""
        if self.parsed_data is None:
            raise ValueError("No data loaded. Call load_data() first.")
        
        output_path = Path(output_dir)
        output_path.mkdir(exist_ok=True)
        
        print("=== Queue Performance Analysis Report ===\n")
        
        # Summary statistics
        print("Implementations tested:")
        for impl in self.parsed_data['implementation'].unique():
            impl_data = self.parsed_data[self.parsed_data['implementation'] == impl]
            thread_range = f"{impl_data['threads'].min()}-{impl_data['threads'].max()}"
            max_throughput = impl_data['throughput_ops_sec'].max()
            print(f"  {impl}: {thread_range} threads, max throughput: {max_throughput:.2e} ops/sec")
        
        print(f"\nThread counts tested: {sorted(self.parsed_data['threads'].unique())}")
        
        # Generate plots
        if save_plots:
            # Throughput scalability
            fig1 = self.plot_scalability('throughput_ops_sec')
            fig1.savefig(output_path / 'throughput_scalability.png', dpi=300, bbox_inches='tight')
            
            # Latency scalability
            fig2 = self.plot_scalability('latency_ns')
            fig2.savefig(output_path / 'latency_scalability.png', dpi=300, bbox_inches='tight')
            
            # Efficiency
            fig3 = self.plot_efficiency()
            fig3.savefig(output_path / 'parallel_efficiency.png', dpi=300, bbox_inches='tight')
            
            # Heatmap
            fig4 = self.plot_heatmap('throughput_ops_sec')
            fig4.savefig(output_path / 'throughput_heatmap.png', dpi=300, bbox_inches='tight')
            
            print(f"\nPlots saved to {output_path}/")
            
            plt.show()

# Example usage
def main():
    """Example usage of the BenchmarkPlotter."""
    
    # Example: Load and visualize benchmark results
    # plotter = BenchmarkPlotter('benchmark_results.json')
    
    # Or create sample data for demonstration
    plotter = BenchmarkPlotter()
    
    # Sample data structure (replace with your actual JSON file)
    sample_data = {
        "benchmarks": [
            {"name": "LockFreeQueue/threads:1", "real_time": 100, "cpu_time": 95, "time_unit": "ns", "iterations": 1000000, "threads": 1},
            {"name": "LockFreeQueue/threads:2", "real_time": 60, "cpu_time": 110, "time_unit": "ns", "iterations": 1000000, "threads": 2},
            {"name": "LockFreeQueue/threads:4", "real_time": 40, "cpu_time": 150, "time_unit": "ns", "iterations": 1000000, "threads": 4},
            {"name": "LockFreeQueue/threads:8", "real_time": 35, "cpu_time": 250, "time_unit": "ns", "iterations": 1000000, "threads": 8},
            {"name": "MutexQueue/threads:1", "real_time": 120, "cpu_time": 115, "time_unit": "ns", "iterations": 1000000, "threads": 1},
            {"name": "MutexQueue/threads:2", "real_time": 150, "cpu_time": 280, "time_unit": "ns", "iterations": 1000000, "threads": 2},
            {"name": "MutexQueue/threads:4", "real_time": 200, "cpu_time": 700, "time_unit": "ns", "iterations": 1000000, "threads": 4},
            {"name": "MutexQueue/threads:8", "real_time": 300, "cpu_time": 2000, "time_unit": "ns", "iterations": 1000000, "threads": 8},
        ]
    }
    
    # Simulate loading sample data
    plotter.raw_data = sample_data
    plotter.data = pd.DataFrame(sample_data['benchmarks'])
    plotter._parse_benchmark_names()
    
    # Generate comprehensive report
    plotter.generate_report()

if __name__ == "__main__":
    # Uncomment to run the example
    # main()
    
    print("Benchmark Plotter ready!")
    print("\nUsage:")
    print("1. Export your Google Benchmark results to JSON:")
    print("   ./your_benchmark --benchmark_format=json --benchmark_out=results.json")
    print("\n2. Use the plotter:")
    print("   plotter = BenchmarkPlotter('results.json')")
    print("   plotter.generate_report()")