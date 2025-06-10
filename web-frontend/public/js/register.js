// 用户注册页面JavaScript

(function() {
    'use strict';

    // DOM元素
    const registerForm = document.getElementById('registerForm');
    const submitButton = registerForm.querySelector('button[type="submit"]');
    const passwordField = document.getElementById('password');
    const confirmPasswordField = document.getElementById('confirmPassword');
    const usernameField = document.getElementById('username');
    const nameField = document.getElementById('name');
    const mailAddrField = document.getElementById('mailAddr');

    // 表单验证函数
    function validateForm() {
        let isValid = true;

        // 验证用户名
        if (!usernameField.value.trim()) {
            showFieldError(usernameField, '请输入用户名');
            isValid = false;
        } else if (usernameField.value.length > 20) {
            showFieldError(usernameField, '用户名长度不能超过20个字符');
            isValid = false;
        } else {
            hideFieldError(usernameField);
        }

        // 验证密码
        if (!passwordField.value) {
            showFieldError(passwordField, '请输入密码');
            isValid = false;
        } else if (passwordField.value.length > 30) {
            showFieldError(passwordField, '密码长度不能超过30个字符');
            isValid = false;
        } else {
            hideFieldError(passwordField);
        }

        // 验证确认密码
        if (!confirmPasswordField.value) {
            showFieldError(confirmPasswordField, '请确认密码');
            isValid = false;
        } else if (passwordField.value !== confirmPasswordField.value) {
            showFieldError(confirmPasswordField, '两次输入的密码不一致');
            isValid = false;
        } else {
            hideFieldError(confirmPasswordField);
        }

        // 验证真实姓名
        if (!nameField.value.trim()) {
            showFieldError(nameField, '请输入真实姓名');
            isValid = false;
        } else if (nameField.value.length > 20) {
            showFieldError(nameField, '真实姓名长度不能超过20个字符');
            isValid = false;
        } else {
            hideFieldError(nameField);
        }

        // 验证邮箱
        if (!mailAddrField.value.trim()) {
            showFieldError(mailAddrField, '请输入邮箱地址');
            isValid = false;
        } else if (!isValidEmail(mailAddrField.value)) {
            showFieldError(mailAddrField, '请输入有效的邮箱地址');
            isValid = false;
        } else if (mailAddrField.value.length > 30) {
            showFieldError(mailAddrField, '邮箱地址长度不能超过30个字符');
            isValid = false;
        } else {
            hideFieldError(mailAddrField);
        }

        return isValid;
    }

    // 显示字段错误
    function showFieldError(field, message) {
        field.classList.add('is-invalid');
        const feedback = field.parentNode.querySelector('.invalid-feedback');
        if (feedback) {
            feedback.textContent = message;
        }
    }

    // 隐藏字段错误
    function hideFieldError(field) {
        field.classList.remove('is-invalid');
    }

    // 验证邮箱格式
    function isValidEmail(email) {
        const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
        return emailRegex.test(email);
    }

    // 显示全局消息
    function showMessage(message, type = 'success') {
        // 移除现有的消息
        const existingAlert = registerForm.querySelector('.alert');
        if (existingAlert) {
            existingAlert.remove();
        }

        // 创建新的消息
        const alertClass = type === 'success' ? 'alert-success' : 'alert-danger';
        const iconClass = type === 'success' ? 'fa-check-circle' : 'fa-exclamation-circle';
        
        const alertDiv = document.createElement('div');
        alertDiv.className = `alert ${alertClass} alert-dismissible fade show`;
        alertDiv.innerHTML = `
            <i class="fas ${iconClass}"></i> ${message}
            <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
        `;

        // 插入到表单开头
        registerForm.insertBefore(alertDiv, registerForm.firstChild);

        // 滚动到消息位置
        alertDiv.scrollIntoView({ behavior: 'smooth', block: 'center' });
    }

    // 设置提交按钮状态
    function setSubmitButtonState(loading = false) {
        if (loading) {
            submitButton.disabled = true;
            submitButton.innerHTML = '<i class="fas fa-spinner fa-spin"></i> 注册中...';
        } else {
            submitButton.disabled = false;
            submitButton.innerHTML = '<i class="fas fa-user-plus"></i> 注册账户';
        }
    }

    // 表单提交处理
    registerForm.addEventListener('submit', async function(e) {
        e.preventDefault();

        // 验证表单
        if (!validateForm()) {
            showMessage('请检查输入信息', 'error');
            return;
        }

        // 检查用户协议
        const agreement = document.getElementById('agreement');
        if (!agreement.checked) {
            showMessage('请先同意用户协议和隐私政策', 'error');
            return;
        }

        try {
            setSubmitButtonState(true);

            // 准备表单数据
            const formData = new FormData(registerForm);
            const data = Object.fromEntries(formData.entries());
            
            // 删除确认密码字段（后端不需要）
            delete data.confirmPassword;

            // 发送注册请求
            const response = await fetch('/api/user/register', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(data)
            });

            const result = await response.json();

            if (result.success) {
                showMessage('注册成功！正在跳转到登录页面...', 'success');
                
                // 延迟跳转，让用户看到成功消息
                setTimeout(() => {
                    window.location.href = '/login?message=' + encodeURIComponent('注册成功，请登录');
                }, 1500);
            } else {
                showMessage(result.message || '注册失败，请重试', 'error');
            }
        } catch (error) {
            console.error('注册错误:', error);
            showMessage('网络错误，请检查网络连接后重试', 'error');
        } finally {
            setSubmitButtonState(false);
        }
    });

    // 实时密码确认验证
    confirmPasswordField.addEventListener('input', function() {
        if (this.value && passwordField.value !== this.value) {
            showFieldError(this, '两次输入的密码不一致');
        } else {
            hideFieldError(this);
        }
    });

    // 用户名输入验证
    usernameField.addEventListener('input', function() {
        const value = this.value.trim();
        if (value && !/^[a-zA-Z][a-zA-Z0-9_]*$/.test(value)) {
            showFieldError(this, '用户名必须以字母开头，只能包含字母、数字和下划线');
        } else if (value.length > 20) {
            showFieldError(this, '用户名长度不能超过20个字符');
        } else {
            hideFieldError(this);
        }
    });

    // 邮箱输入验证
    mailAddrField.addEventListener('input', function() {
        const value = this.value.trim();
        if (value && !isValidEmail(value)) {
            showFieldError(this, '请输入有效的邮箱地址');
        } else if (value.length > 30) {
            showFieldError(this, '邮箱地址长度不能超过30个字符');
        } else {
            hideFieldError(this);
        }
    });

    // 页面加载完成后的处理
    document.addEventListener('DOMContentLoaded', function() {
        // 如果URL中有错误参数，显示错误信息
        const urlParams = new URLSearchParams(window.location.search);
        const error = urlParams.get('error');
        if (error) {
            showMessage(decodeURIComponent(error), 'error');
        }

        // 聚焦到第一个输入框
        usernameField.focus();
    });

})();
