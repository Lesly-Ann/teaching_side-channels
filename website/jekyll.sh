#!/bin/bash
# Jekyll build script for lab documentation

set -e

LAB_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$LAB_DIR"

echo "🔨 Jekyll Build Script"
echo "======================"
echo ""

# Check if Jekyll is installed
if ! command -v jekyll &> /dev/null; then
    echo "⚠️  Jekyll not found. Installing..."
    echo ""
    
    # Check if Ruby is installed
    if ! command -v ruby &> /dev/null; then
        echo "❌ Ruby is not installed. Please install Ruby first:"
        echo ""
        echo "  Ubuntu/Debian: sudo apt-get install ruby-full build-essential"
        echo "  macOS:         brew install ruby"
        echo "  Or visit:      https://www.ruby-lang.org/en/documentation/installation/"
        echo ""
        exit 1
    fi
    
    echo "📦 Installing bundler..."
    gem install bundler
    
    echo "📦 Installing Jekyll and dependencies..."
    bundle install
    echo ""
fi

# Parse command line arguments
COMMAND=${1:-build}

case $COMMAND in
    build)
        echo "🔨 Building site..."
        bundle exec jekyll build
        echo ""
        echo "✅ Build complete!"
        echo "📁 Output: _site/index.html"
        echo ""
        echo "To view: firefox _site/index.html"
        ;;
        
    serve)
        echo "🚀 Starting Jekyll server..."
        echo "📡 Server will be available at: http://localhost:4000"
        echo "Press Ctrl+C to stop"
        echo ""
        bundle exec jekyll serve --livereload
        ;;
        
    watch)
        echo "👀 Building with watch mode..."
        echo "Press Ctrl+C to stop"
        echo ""
        bundle exec jekyll build --watch
        ;;
        
    clean)
        echo "🧹 Cleaning generated files..."
        bundle exec jekyll clean
        rm -rf .jekyll-cache
        echo "✅ Clean complete!"
        ;;
        
    install)
        echo "📦 Installing dependencies..."
        bundle install
        echo "✅ Dependencies installed!"
        ;;
        
    *)
        echo "Usage: $0 [command]"
        echo ""
        echo "Commands:"
        echo "  build   - Build the site (default)"
        echo "  serve   - Build and serve with live reload"
        echo "  watch   - Build and watch for changes"
        echo "  clean   - Remove generated files"
        echo "  install - Install dependencies"
        echo ""
        echo "Examples:"
        echo "  $0              # Build the site"
        echo "  $0 serve        # Start dev server"
        echo "  $0 clean        # Clean build files"
        ;;
esac
