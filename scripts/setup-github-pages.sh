#!/bin/bash

# GitHub Pages Setup Script for Shell Project
# This script provides instructions for enabling GitHub Pages with the performance dashboard

set -e

echo "🚀 GitHub Pages Setup for Shell Project Performance Dashboard"
echo "============================================================="
echo

echo "📋 Setup Checklist:"
echo

echo "1. Enable GitHub Pages:"
echo "   - Go to: https://github.com/wsollers/shell/settings/pages"
echo "   - Under 'Source', select 'GitHub Actions'"
echo "   - Click 'Save'"
echo

echo "2. Set Required Repository Secrets:"
echo "   - Go to: https://github.com/wsollers/shell/settings/secrets/actions"
echo "   - Add the following secrets:"
echo "     * CODECOV_TOKEN: (Get from codecov.io)"
echo

echo "3. Optional Azure Integration Secrets (for Defender for DevOps):"
echo "     * AZURE_CLIENT_ID: Service principal client ID"
echo "     * AZURE_CLIENT_SECRET: Service principal secret"
echo "     * AZURE_TENANT_ID: Azure tenant ID"
echo "     * AZURE_SUBSCRIPTION_ID: Azure subscription ID"
echo

echo "4. Trigger Initial Deployment:"
echo "   - Push a commit to main branch or create a pull request"
echo "   - Monitor the 'Actions' tab for workflow completion"
echo "   - Dashboard will be available at: https://wsollers.github.io/shell/"
echo

echo "✅ Verification Steps:"
echo "1. Check that GitHub Pages is enabled: https://github.com/wsollers/shell/settings/pages"
echo "2. Verify workflows run successfully: https://github.com/wsollers/shell/actions"
echo "3. Visit the dashboard: https://wsollers.github.io/shell/"
echo

echo "🔧 Troubleshooting:"
echo "- If workflows fail, check the 'Actions' tab for error details"
echo "- Ensure GitHub Pages source is set to 'GitHub Actions'"
echo "- Verify all required secrets are configured"
echo

echo "📊 Dashboard Features:"
echo "- Real-time performance benchmarks"
echo "- Interactive code coverage reports"
echo "- Auto-generated API documentation"
echo "- Fuzzing and security test results"
echo

read -p "Press Enter to open GitHub Pages settings..." -r
if command -v xdg-open >/dev/null; then
    xdg-open "https://github.com/wsollers/shell/settings/pages"
elif command -v open >/dev/null; then
    open "https://github.com/wsollers/shell/settings/pages"
else
    echo "Please manually navigate to: https://github.com/wsollers/shell/settings/pages"
fi