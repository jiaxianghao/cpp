#!/usr/bin/env python3
"""
IDL Template Generator
Interactive tool to create new IDL files with common patterns
"""

import os
import sys
import argparse
from datetime import datetime

# IDL templates
SIMPLE_STRUCT_TEMPLATE = """// {comment}
// Generated on {date}

struct {name}
{{
    string id;
    long timestamp;
}};
"""

SENSOR_TEMPLATE = """// Sensor data message
// Generated on {date}

struct {name}
{{
    string sensor_id;
    string sensor_type;
    double value;
    string unit;
    long timestamp;
    boolean is_valid;
}};
"""

COMMAND_TEMPLATE = """// Command message
// Generated on {date}

enum {name}Type
{{
    START,
    STOP,
    PAUSE,
    RESUME
}};

struct {name}
{{
    string command_id;
    {name}Type command_type;
    string target_id;
    string parameters;
    long timestamp;
}};
"""

COMPLEX_TEMPLATE = """// Complex nested structure
// Generated on {date}

struct Position
{{
    double x;
    double y;
    double z;
}};

struct {name}
{{
    string id;
    Position position;
    sequence<double> values;
    long timestamp;
}};
"""

def create_idl_file(name, template_type, output_dir, comment="Auto-generated IDL file"):
    """Create an IDL file from template"""
    
    templates = {
        'simple': SIMPLE_STRUCT_TEMPLATE,
        'sensor': SENSOR_TEMPLATE,
        'command': COMMAND_TEMPLATE,
        'complex': COMPLEX_TEMPLATE
    }
    
    if template_type not in templates:
        print(f"Error: Unknown template type '{template_type}'")
        return False
    
    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)
    
    # Generate filename
    filename = os.path.join(output_dir, f"{name}.idl")
    
    if os.path.exists(filename):
        response = input(f"File {filename} already exists. Overwrite? (y/n): ")
        if response.lower() != 'y':
            print("Cancelled.")
            return False
    
    # Fill template
    content = templates[template_type].format(
        name=name,
        comment=comment,
        date=datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    )
    
    # Write file
    with open(filename, 'w') as f:
        f.write(content)
    
    print(f"✓ Created {filename}")
    return True

def generate_example_code(name):
    """Generate example C++ code for using the new type"""
    
    example = f"""
// Example code for using {name}

#include "dds_wrapper/DDSManager.h"
#include "{name}.h"
#include <iostream>

using namespace dds_wrapper;

int main()
{{
    // Initialize
    DDSManager manager;
    manager.initialize();
    
    // Create publisher
    auto pub = manager.createPublisher<{name}>("{name}Topic");
    
    // Create subscriber
    auto sub = manager.createSubscriber<{name}>("{name}Topic",
        [](const {name}& msg)
        {{
            std::cout << "Received message" << std::endl;
            // Access message fields here
        }});
    
    // Publish a message
    {name} msg;
    msg.id("MSG_001");
    msg.timestamp(std::chrono::system_clock::now().time_since_epoch().count());
    
    pub->publish(msg);
    
    return 0;
}}
"""
    
    return example

def interactive_mode():
    """Interactive mode to create IDL files"""
    
    print("=" * 60)
    print("FastDDS Wrapper - IDL Template Generator")
    print("=" * 60)
    print()
    
    # Get name
    name = input("Enter message type name (e.g., RobotStatus): ").strip()
    if not name:
        print("Error: Name cannot be empty")
        return False
    
    # Get template type
    print("\nAvailable templates:")
    print("  1. simple  - Basic struct with id and timestamp")
    print("  2. sensor  - Sensor data with value, unit, etc.")
    print("  3. command - Command message with enum type")
    print("  4. complex - Complex nested structure")
    print()
    
    choice = input("Select template (1-4): ").strip()
    template_map = {'1': 'simple', '2': 'sensor', '3': 'command', '4': 'complex'}
    
    if choice not in template_map:
        print("Error: Invalid choice")
        return False
    
    template_type = template_map[choice]
    
    # Get comment
    comment = input("\nEnter description (optional): ").strip()
    if not comment:
        comment = f"{name} message type"
    
    # Get output directory
    default_dir = "../idl" if os.path.exists("../idl") else "idl"
    output_dir = input(f"\nOutput directory (default: {default_dir}): ").strip()
    if not output_dir:
        output_dir = default_dir
    
    print()
    print("Creating IDL file...")
    
    # Create file
    if create_idl_file(name, template_type, output_dir, comment):
        print()
        print("✓ IDL file created successfully!")
        print()
        print("Next steps:")
        print("  1. Review the generated IDL file")
        print("  2. Run: cd build && cmake .. && make")
        print("  3. Use the type in your code")
        print()
        
        show_example = input("Show example C++ code? (y/n): ").strip()
        if show_example.lower() == 'y':
            print()
            print("=" * 60)
            print("Example C++ Code:")
            print("=" * 60)
            print(generate_example_code(name))
        
        return True
    
    return False

def main():
    parser = argparse.ArgumentParser(description='Generate IDL files from templates')
    parser.add_argument('--name', help='Message type name')
    parser.add_argument('--type', choices=['simple', 'sensor', 'command', 'complex'],
                       help='Template type')
    parser.add_argument('--output', default='../idl', help='Output directory')
    parser.add_argument('--comment', help='Description comment')
    parser.add_argument('--interactive', '-i', action='store_true',
                       help='Run in interactive mode')
    
    args = parser.parse_args()
    
    if args.interactive or (not args.name or not args.type):
        # Interactive mode
        interactive_mode()
    else:
        # Command line mode
        comment = args.comment if args.comment else f"{args.name} message type"
        create_idl_file(args.name, args.type, args.output, comment)

if __name__ == '__main__':
    main()
