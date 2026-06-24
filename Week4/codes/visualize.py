import json
import sys
import argparse

def compare_results(file1, file2):
    try:
        with open(file1, 'r') as f1, open(file2, 'r') as f2:
            data1 = json.load(f1)
            data2 = json.load(f2)
            
        res1 = data1.get("results", [])
        res2 = data2.get("results", [])
        
        if len(res1) != len(res2):
            print(f"Warning: Result counts differ! ({len(res1)} vs {len(res2)})")
            
        print(f"{'Query ID':<10} | {'Brute Force':<15} | {'Optimized':<15} | {'Diff / Ratio':<15}")
        print("-" * 60)
        
        for i in range(min(len(res1), len(res2))):
            id1 = res1[i].get("query_id", i)
            cost1 = res1[i].get("cost", "N/A")
            cost2 = res2[i].get("cost", "N/A")
            
            diff = ""
            if isinstance(cost1, (int, float)) and isinstance(cost2, (int, float)):
                if cost1 == cost2:
                    diff = "Exact Match ✅"
                else:
                    ratio = (cost2 / cost1) if cost1 != 0 else float('inf')
                    diff = f"{ratio:.2f}x OPT"
            
            print(f"{id1:<10} | {str(cost1):<15} | {str(cost2):<15} | {diff:<15}")
            
    except Exception as e:
        print(f"Error comparing files: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compare TSP output JSONs.")
    parser.add_argument("brute_file", help="JSON output from the brute force solver")
    parser.add_argument("opt_file", help="JSON output from the optimized solver")
    args = parser.parse_args()
    
    print(f"Comparing {args.brute_file} against {args.opt_file}\n")
    compare_results(args.brute_file, args.opt_file)
